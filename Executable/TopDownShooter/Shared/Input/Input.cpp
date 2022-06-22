#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Input/Input.hpp>
#include <Input/InputListenerComponent.hpp>
#include <Input/InputSingleton.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>

namespace Input
{
/// \brief Contains common logic and common queries for input dispatchers.
class InputDispatcherBase
{
public:
    InputDispatcherBase (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

protected:
    /// \breif We need to clean actions in all listeners, otherwise old data might be left in unsubscribed listeners.
    void ClearListeners () noexcept;

    void DispatchActions (InputSingleton::SubscriptionVector &_subscribers,
                          const InputSingleton::ActionsBuffer &_buffer) noexcept;

    Emergence::Celerity::ModifySingletonQuery modifyInput;
    Emergence::Celerity::EditValueQuery editListenerById;
    Emergence::Celerity::EditAscendingRangeQuery editListeners;
};

InputDispatcherBase::InputDispatcherBase (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyInput (MODIFY_SINGLETON (InputSingleton)),
      editListenerById (EDIT_VALUE_1F (InputListenerComponent, objectId)),
      editListeners (EDIT_ASCENDING_RANGE (InputListenerComponent, objectId))
{
    _constructor.DependOn (Checkpoint::INPUT_DISPATCH_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::INPUT_LISTENERS_PUSH_ALLOWED);
    // Enforce checkpoint order even if there are no direct pushers.
    _constructor.MakeDependencyOf (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);
}

void InputDispatcherBase::ClearListeners () noexcept
{
    auto cursor = editListeners.Execute (nullptr, nullptr);
    while (*cursor)
    {
        auto *listener = static_cast<InputListenerComponent *> (*cursor);
        listener->actions.Clear ();
        ++cursor;
    }
}

void InputDispatcherBase::DispatchActions (InputSingleton::SubscriptionVector &_subscribers,
                                           const InputSingleton::ActionsBuffer &_buffer) noexcept
{
    if (_buffer.Empty ())
    {
        return;
    }

    for (auto iterator = _subscribers.Begin (); iterator != _subscribers.End ();)
    {
        const InputSubscription &subscription = *iterator;
        auto listenerCursor = editListenerById.Execute (&subscription.listenerId);
        auto *listener = static_cast<InputListenerComponent *> (*listenerCursor);

        if (!listener)
        {
            iterator = _subscribers.EraseExchangingWithLast (iterator);
            continue;
        }

        for (const InputAction &action : _buffer)
        {
            if (subscription.group == action.group && listener->actions.GetCount () < listener->actions.GetCapacity ())
            {
                listener->actions.EmplaceBack (action);
            }
        }

        ++iterator;
    }
}

/// \brief Input dispatcher for fixed update.
/// \details Does not capture any events (as it makes no sense for fixed update), only dispatches them.
class FixedInputDispatcher final : public Emergence::Celerity::TaskExecutorBase<FixedInputDispatcher>,
                                   public InputDispatcherBase

{
public:
    FixedInputDispatcher (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;
};

FixedInputDispatcher::FixedInputDispatcher (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : InputDispatcherBase (_constructor)
{
}

void FixedInputDispatcher::Execute () noexcept
{
    auto cursor = modifyInput.Execute ();
    auto *input = static_cast<InputSingleton *> (*cursor);

    ClearListeners ();
    DispatchActions (input->fixedSubscriptions, input->fixedPersistentActionsBuffer);
    DispatchActions (input->fixedSubscriptions, input->fixedInstantActionsBuffer);
    input->fixedInstantActionsBuffer.Clear ();
}

/// \brief Input dispatcher for normal update.
/// \details Captures input and routes it not only to normal input subscribers, but to fixed buffers too.
class NormalInputDispatcher final : public Emergence::Celerity::TaskExecutorBase<NormalInputDispatcher>,
                                    public InputDispatcherBase
{
public:
    NormalInputDispatcher (Emergence::Celerity::TaskConstructor &_constructor,
                           Emergence::Celerity::InputAccumulator *_inputAccumulator) noexcept;

    void Execute () noexcept;

private:
    static void SendAction (InputSingleton *_input, const InputAction &_action, bool _instant) noexcept;

    void UpdateActionBuffers (InputSingleton *_input) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::InputAccumulator *inputAccumulator = nullptr;
};

NormalInputDispatcher::NormalInputDispatcher (Emergence::Celerity::TaskConstructor &_constructor,
                                              Emergence::Celerity::InputAccumulator *_inputAccumulator) noexcept
    : InputDispatcherBase (_constructor),
      fetchWorld (_constructor.FetchSingleton (Emergence::Celerity::WorldSingleton::Reflect ().mapping)),
      inputAccumulator (_inputAccumulator)
{
    assert (inputAccumulator);
}

void NormalInputDispatcher::Execute () noexcept
{
    auto inputCursor = modifyInput.Execute ();
    auto *input = static_cast<InputSingleton *> (*inputCursor);

    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    if (world->fixedUpdateHappened)
    {
        // Clear persistent events, that were passed from previous normal update to fixed update.
        input->fixedPersistentActionsBuffer.DropLeading (input->accumulatedPersistentActionsForFixedUpdate);
    }

    ClearListeners ();
    UpdateActionBuffers (input);
    DispatchActions (input->normalSubscriptions, input->normalActionsBuffer);
    input->normalActionsBuffer.Clear ();
}

void NormalInputDispatcher::SendAction (InputSingleton *_input, const InputAction &_action, bool _instant) noexcept
{
    // Only instant actions can be duplicated.
    if (_instant || _input->normalActionsBuffer.Find (_action) == _input->normalActionsBuffer.End ())
    {
        _input->normalActionsBuffer.TryEmplaceBack (_action);
    }

    if (_instant)
    {
        _input->fixedInstantActionsBuffer.TryEmplaceBack (_action);
    }
    // Persistent actions should not be duplicated.
    else if (_input->fixedPersistentActionsBuffer.Find (_action) == _input->fixedPersistentActionsBuffer.End ())
    {
        _input->fixedPersistentActionsBuffer.TryEmplaceBack (_action);
    }
}

void NormalInputDispatcher::UpdateActionBuffers (InputSingleton *_input) noexcept
{
    for (KeyStateTrigger &trigger : _input->keyStateTriggers)
    {
        // Can not be triggered more than once per frame.
        bool canBeTriggered = true;

        for (const Emergence::Celerity::InputEvent &event : inputAccumulator->GetAccumulatedEvents ())
        {
            if (event.type == Emergence::Celerity::InputType::KEYBOARD && trigger.scan == event.keyboard.scan)
            {
                trigger.isDownNow = event.keyboard.down;
                if (canBeTriggered && trigger.isDownNow == trigger.down)
                {
                    canBeTriggered = false;
                    SendAction (_input, trigger.action, false);
                }
            }
        }

        if (canBeTriggered && trigger.isDownNow == trigger.down)
        {
            SendAction (_input, trigger.action, false);
        }
    }

    for (KeyStateChangedTrigger &trigger : _input->keyStateChangedTriggers)
    {
        for (const Emergence::Celerity::InputEvent &event : inputAccumulator->GetAccumulatedEvents ())
        {
            if (event.type == Emergence::Celerity::InputType::KEYBOARD && trigger.scan == event.keyboard.scan &&
                event.keyboard.down == trigger.pressed && event.keyboard.qualifiers == trigger.qualifiers)
            {
                SendAction (_input, trigger.action, true);
            }
        }
    }

    _input->accumulatedPersistentActionsForFixedUpdate = _input->fixedPersistentActionsBuffer.GetCount ();
}

using namespace Emergence::Memory::Literals;

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Input::RemoveFixedListeners"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Transform::Transform3dComponentRemovedFixedEvent, InputListenerComponent,
                                objectId)
        .MakeDependencyOf ("Input::FixedDispatcher"_us);

    _pipelineBuilder.AddTask ("Input::FixedDispatcher"_us).SetExecutor<FixedInputDispatcher> ();
}

void AddToNormalUpdate (Emergence::Celerity::InputAccumulator *_inputAccumulator,
                        Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Input::RemoveNormalListeners"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Transform::Transform3dComponentRemovedNormalEvent, InputListenerComponent,
                                objectId)
        .MakeDependencyOf ("Input::NormalDispatcher"_us);

    _pipelineBuilder.AddTask ("Input::NormalDispatcher"_us).SetExecutor<NormalInputDispatcher> (_inputAccumulator);
}
} // namespace Input
