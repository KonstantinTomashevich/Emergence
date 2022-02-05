#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/Model/WorldSingleton.hpp>

#include <Input/Input.hpp>
#include <Input/InputListenerObject.hpp>
#include <Input/InputSingleton.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

#include <Shared/Checkpoint.hpp>

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

    Emergence::Warehouse::ModifySingletonQuery modifyInput;
    Emergence::Warehouse::ModifyValueQuery modifyListenersById;
    Emergence::Warehouse::ModifyAscendingRangeQuery modifyListeners;
};

InputDispatcherBase::InputDispatcherBase (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyInput (_constructor.ModifySingleton (InputSingleton::Reflect ().mapping)),
      modifyListenersById (_constructor.ModifyValue (InputListenerObject::Reflect ().mapping,
                                                     {InputListenerObject::Reflect ().objectId})),
      modifyListeners (_constructor.ModifyAscendingRange (InputListenerObject::Reflect ().mapping,
                                                          InputListenerObject::Reflect ().objectId))
{
    _constructor.DependOn (Checkpoint::INPUT_DISPATCH_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::INPUT_LISTENERS_PUSH_ALLOWED);
    // In case if there are no direct pushers.
    _constructor.MakeDependencyOf (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);
}

void InputDispatcherBase::ClearListeners () noexcept
{
    auto cursor = modifyListeners.Execute (nullptr, nullptr);
    while (*cursor)
    {
        auto *listener = static_cast<InputListenerObject *> (*cursor);
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

    for (auto iterator = _subscribers.Begin (); iterator != _subscribers.End (); ++iterator)
    {
        const InputSubscription &subscription = *iterator;
        auto listenerCursor = modifyListenersById.Execute (&subscription.listenerId);
        auto *listener = static_cast<InputListenerObject *> (*listenerCursor);

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

    Emergence::Warehouse::FetchSingletonQuery fetchWorld;
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
            if (event.type == Emergence::Celerity::InputType::KEYBOARD && trigger.key == event.keyboard.key)
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
            if (event.type == Emergence::Celerity::InputType::KEYBOARD && trigger.key == event.keyboard.key &&
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
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("FixedInputDispatcher"_us);
    constructor.SetExecutor<FixedInputDispatcher> ();
}

void AddToNormalUpdate (Emergence::Celerity::InputAccumulator *_inputAccumulator,
                        Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("NormalInputDispatcher"_us);
    constructor.SetExecutor<NormalInputDispatcher> (_inputAccumulator);
}
} // namespace Input
