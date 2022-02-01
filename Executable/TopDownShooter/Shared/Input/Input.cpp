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
    void UpdateTriggers (InputSingleton *_input,
                         InputSingleton::KeyboardTriggerVector &_triggers,
                         bool _instant) noexcept;

    void UpdateActionBuffers (InputSingleton *_input) noexcept;

    Emergence::Warehouse::FetchSingletonQuery fetchWorld;
    Emergence::Container::Vector<KeyboardEvent> postponedEvents {Emergence::Memory::Profiler::AllocationGroup::Top ()};

    // Last received state of qualifiers. Used to check if persistent triggers can be activated without new events.
    QualifiersMask lastQualifiersState;
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
        lastQualifiersState = static_cast<QualifiersMask> (_event.keysym.mod);
        postponedEvents.emplace_back (KeyboardEvent {static_cast<KeyCode> (_event.keysym.sym),
                                                     static_cast<QualifiersMask> (_event.keysym.mod), true});
    }

    return InputListener::keyPressed (_event);
}

bool NormalInputDispatcher::keyReleased (const OgreBites::KeyboardEvent &_event)
{
    if (!_event.repeat)
    {
        lastQualifiersState = static_cast<QualifiersMask> (_event.keysym.mod);
        postponedEvents.emplace_back (KeyboardEvent {static_cast<KeyCode> (_event.keysym.sym),
                                                     static_cast<QualifiersMask> (_event.keysym.mod), false});
    }

    return InputListener::keyPressed (_event);
}

void NormalInputDispatcher::UpdateTriggers (InputSingleton *_input,
                                            InputSingleton::KeyboardTriggerVector &_triggers,
                                            bool _instant) noexcept
{
    for (KeyboardActionTrigger &trigger : _triggers)
    {
        bool canBeTriggered = true;
        const uint32_t satisfactionMask = (1u << trigger.keys.GetCount ()) - 1u;

        auto checkTrigger =
            [_input, _instant, satisfactionMask, &canBeTriggered, &trigger] (QualifiersMask _currentQualifiers)
        {
            if (canBeTriggered && trigger.keysState == satisfactionMask && _currentQualifiers == trigger.qualifiers)
            {
                // Only instant actions can be duplicated.
                if (_instant || _input->normalActionsBuffer.Find (trigger.action) == _input->normalActionsBuffer.End ())
                {
                    _input->normalActionsBuffer.TryEmplaceBack (trigger.action);
                }

                if (_instant)
                {
                    _input->fixedInstantActionsBuffer.TryEmplaceBack (trigger.action);
                }
                // Persistent actions should not be duplicated.
                else if (_input->fixedPersistentActionsBuffer.Find (trigger.action) ==
                         _input->fixedPersistentActionsBuffer.End ())
                {
                    _input->fixedPersistentActionsBuffer.TryEmplaceBack (trigger.action);
                }

                canBeTriggered = _instant;
            }
        };

        for (const KeyboardEvent &event : postponedEvents)
        {
            for (std::size_t index = 0u; index < trigger.keys.GetCount (); ++index)
            {
                if (trigger.keys[index] == event.key)
                {
                    if (event.down)
                    {
                        trigger.keysState |= 1u << index;
                    }
                    else
                    {
                        trigger.keysState &= ~(1u << index);
                    }

                    checkTrigger (event.qualifiers);
                    break;
                }
            }
        }

        // Persistent event might not receive any changes from events, but still be fired.
        // For example: player holds attack button.
        if (!_instant)
        {
            checkTrigger (lastQualifiersState);
        }

        if (_instant)
        {
            // Instant event requires buttons to be pressed during single frame.
            trigger.keysState = 0u;
        }
    }
}

void NormalInputDispatcher::UpdateActionBuffers (InputSingleton *_input) noexcept
{
    UpdateTriggers (_input, _input->keyboardInstantTriggers, true);
    UpdateTriggers (_input, _input->keyboardPersistentTriggers, false);
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
