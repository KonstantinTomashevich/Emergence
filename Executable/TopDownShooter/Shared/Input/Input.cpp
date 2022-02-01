#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/Model/WorldSingleton.hpp>

#include <Input/Input.hpp>
#include <Input/InputListenerObject.hpp>
#include <Input/InputSingleton.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

BEGIN_MUTING_WARNINGS
#include <OgreInput.h>
END_MUTING_WARNINGS

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
    DispatchActions (input->fixedSubscriptions, input->fixedInstantActionsBuffer);
    DispatchActions (input->fixedSubscriptions, input->fixedPersistentActionsBuffer);
    input->fixedInstantActionsBuffer.Clear ();
}

/// \brief Postponed event info.
/// \details NormalInputDispatcher receives events from Urho3D and postpones processing till execution.
struct KeyboardEvent final
{
    KeyCode key;
    QualifiersMask qualifiers;
    bool down;
};

/// \brief Input dispatcher for normal update.
/// \details Captures input and routes it not only to normal input subscribers, but to fixed buffers too.
class NormalInputDispatcher final : public Emergence::Celerity::TaskExecutorBase<NormalInputDispatcher>,
                                    public InputDispatcherBase,
                                    public OgreBites::InputListener
{
public:
    NormalInputDispatcher (Emergence::Celerity::TaskConstructor &_constructor,
                           OgreBites::ApplicationContextBase *_application) noexcept;

    NormalInputDispatcher (const NormalInputDispatcher &_other) = delete;

    NormalInputDispatcher (NormalInputDispatcher &&_other) = delete;

    ~NormalInputDispatcher () noexcept final;

    void Execute () noexcept;

    bool keyPressed (const OgreBites::KeyboardEvent &_event) override;

    bool keyReleased (const OgreBites::KeyboardEvent &_event) override;

    EMERGENCE_DELETE_ASSIGNMENT (NormalInputDispatcher);

private:
    static void SendAction (InputSingleton *_input, const InputAction &_action, bool _instant) noexcept;

    void UpdateActionBuffers (InputSingleton *_input) noexcept;

    Emergence::Warehouse::FetchSingletonQuery fetchWorld;
    Emergence::Container::Vector<KeyboardEvent> postponedEvents {Emergence::Memory::Profiler::AllocationGroup::Top ()};
    OgreBites::ApplicationContextBase *application;
};

NormalInputDispatcher::NormalInputDispatcher (Emergence::Celerity::TaskConstructor &_constructor,
                                              OgreBites::ApplicationContextBase *_application) noexcept
    : InputDispatcherBase (_constructor),
      fetchWorld (_constructor.FetchSingleton (Emergence::Celerity::WorldSingleton::Reflect ().mapping)),
      application (_application)
{
    application->addInputListener (this);
}

NormalInputDispatcher::~NormalInputDispatcher () noexcept
{
    application->removeInputListener (this);
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

bool NormalInputDispatcher::keyPressed (const OgreBites::KeyboardEvent &_event)
{
    if (!_event.repeat)
    {
        postponedEvents.emplace_back (KeyboardEvent {static_cast<KeyCode> (_event.keysym.sym),
                                                     static_cast<QualifiersMask> (_event.keysym.mod), true});
    }

    return InputListener::keyPressed (_event);
}

bool NormalInputDispatcher::keyReleased (const OgreBites::KeyboardEvent &_event)
{
    if (!_event.repeat)
    {
        postponedEvents.emplace_back (KeyboardEvent {static_cast<KeyCode> (_event.keysym.sym),
                                                     static_cast<QualifiersMask> (_event.keysym.mod), false});
    }

    return InputListener::keyPressed (_event);
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

        for (const KeyboardEvent &event : postponedEvents)
        {
            if (trigger.key == event.key)
            {
                trigger.isDownNow = event.down;
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
        for (const KeyboardEvent &event : postponedEvents)
        {
            if (trigger.key == event.key && event.down == trigger.pressed && event.qualifiers == trigger.qualifiers)
            {
                SendAction (_input, trigger.action, true);
            }
        }
    }

    _input->accumulatedPersistentActionsForFixedUpdate = _input->fixedPersistentActionsBuffer.GetCount ();
    postponedEvents.clear ();
}

using namespace Emergence::Memory::Literals;

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("FixedInputDispatcher"_us);
    constructor.SetExecutor<FixedInputDispatcher> ();
}

void AddToNormalUpdate (OgreBites::ApplicationContextBase *_application,
                        Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("NormalInputDispatcher"_us);
    constructor.SetExecutor<NormalInputDispatcher> (_application);
}
} // namespace Input
