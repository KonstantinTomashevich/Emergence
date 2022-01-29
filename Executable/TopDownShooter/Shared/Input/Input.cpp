#include <SyntaxSugar/MuteWarnings.hpp>

#include <cassert>

#include <Input/Input.hpp>
#include <Input/InputListenerObject.hpp>
#include <Input/InputSingleton.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

#include <Shared/Checkpoint.hpp>
#include <Shared/WorldInfoSingleton.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/Core/Context.h>
END_MUTING_WARNINGS

namespace InputCollection
{
/// \brief Contains common logic and common queries for input dispatchers.
class InputDispatcherBase
{
public:
    InputDispatcherBase (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

protected:
    /// \breif We need to clean actions in all listeners, otherwise old data might be left in unsubscribed listeners.
    void ClearListeners () noexcept;

    void DispatchActions (const InputSingleton::SubscriptionVector &_subscribers,
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

void InputDispatcherBase::DispatchActions (const InputSingleton::SubscriptionVector &_subscribers,
                                           const InputSingleton::ActionsBuffer &_buffer) noexcept
{
    if (_buffer.Empty ())
    {
        return;
    }

    for (const InputSubscription &subscription : _subscribers)
    {
        auto listenerCursor = modifyListenersById.Execute (&subscription.listenerId);
        auto *listener = static_cast<InputListenerObject *> (*listenerCursor);

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
class FixedInputDispatcher final : public InputDispatcherBase,
                                   public Emergence::Celerity::TaskExecutorBase<FixedInputDispatcher>
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
class NormalInputDispatcher final : public InputDispatcherBase,
                                    public Emergence::Celerity::TaskExecutorBase<NormalInputDispatcher>,
                                    Urho3D::Object
{
    URHO3D_OBJECT (NormalInputDispatcher, Object)

public:
    NormalInputDispatcher (Emergence::Celerity::TaskConstructor &_constructor, Urho3D::Context *_context) noexcept;

    void Execute () noexcept;

private:
    void UpdateTriggers (InputSingleton *_input,
                         InputSingleton::KeyboardTriggerVector &_triggers,
                         bool _instant) noexcept;

    void UpdateActionBuffers (InputSingleton *_input) noexcept;

    Emergence::Warehouse::FetchSingletonQuery fetchWorldInfo;
    Emergence::Container::Vector<KeyboardEvent> postponedEvents {Emergence::Memory::Profiler::AllocationGroup::Top ()};
};

NormalInputDispatcher::NormalInputDispatcher (Emergence::Celerity::TaskConstructor &_constructor,
                                              Urho3D::Context *_context) noexcept
    : InputDispatcherBase (_constructor),
      Object (_context),
      fetchWorldInfo (_constructor.FetchSingleton (WorldInfoSingleton::Reflect ().mapping))
{
    SubscribeToEvent (Urho3D::E_KEYDOWN,
                      [this] (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_data)
                      {
                          if (_data[Urho3D::KeyDown::P_REPEAT].GetBool ())
                          {
                              return;
                          }

                          postponedEvents.emplace_back (KeyboardEvent {
                              static_cast<KeyCode> (_data[Urho3D::KeyDown::P_KEY].GetInt ()),
                              static_cast<QualifiersMask> (_data[Urho3D::KeyDown::P_QUALIFIERS].GetUInt ()), true});
                      });

    SubscribeToEvent (Urho3D::E_KEYUP,
                      [this] (Urho3D::StringHash /*unused*/, Urho3D::VariantMap &_data)
                      {
                          postponedEvents.emplace_back (KeyboardEvent {
                              static_cast<KeyCode> (_data[Urho3D::KeyDown::P_KEY].GetInt ()),
                              static_cast<QualifiersMask> (_data[Urho3D::KeyDown::P_QUALIFIERS].GetUInt ()), false});
                      });
}

void NormalInputDispatcher::Execute () noexcept
{
    auto inputCursor = modifyInput.Execute ();
    auto *input = static_cast<InputSingleton *> (*inputCursor);

    auto worldInfoCursor = fetchWorldInfo.Execute ();
    const auto *worldInfo = static_cast<const WorldInfoSingleton *> (*worldInfoCursor);

    if (worldInfo->fixedUpdateHappened)
    {
        // Clear persistent events, that were passed from previous normal update to fixed update.
        input->fixedPersistentActionsBuffer.DropLeading (input->accumulatedPersistentActionsForFixedUpdate);
    }

    ClearListeners ();
    UpdateActionBuffers (input);
    DispatchActions (input->normalSubscriptions, input->normalActionsBuffer);
    input->normalActionsBuffer.Clear ();
}

void NormalInputDispatcher::UpdateTriggers (InputSingleton *_input,
                                            InputSingleton::KeyboardTriggerVector &_triggers,
                                            bool _instant) noexcept
{
    for (KeyboardActionTrigger &trigger : _triggers)
    {
        bool canBeTriggered = true;
        const uint32_t satisfactionMask = (1u << trigger.keys.GetCount ()) - 1u;

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

                    if (canBeTriggered && trigger.keysState == satisfactionMask &&
                        event.qualifiers == trigger.qualifiers)
                    {
                        // Only instant actions can be duplicated.
                        if (_instant ||
                            _input->normalActionsBuffer.Find (trigger.action) == _input->normalActionsBuffer.End ())
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

                    break;
                }
            }
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

void AddToNormalUpdate (Urho3D::Context *_context, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("NormalInputDispatcher"_us);
    constructor.SetExecutor<NormalInputDispatcher> (_context);
}
} // namespace InputCollection
