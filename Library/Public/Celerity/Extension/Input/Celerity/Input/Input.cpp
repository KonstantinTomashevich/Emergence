#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputActionComponent.hpp>
#include <Celerity/Input/InputActionHolder.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
#include <Celerity/Input/InputTriggers.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>

namespace Emergence::Celerity::Input
{
const Memory::UniqueString Checkpoint::ACTION_DISPATCH_STARTED {"Input::ActionDispatchStarted"};

const Memory::UniqueString Checkpoint::CUSTOM_ACTION_COMPONENT_INSERT_ALLOWED {
    "Input::CustomActionComponentInsertAllowed"};

const Memory::UniqueString Checkpoint::ACTION_COMPONENT_READ_ALLOWED {"Input::ActionComponentReadAllowed"};

class InputProcessorBase
{
public:
    InputProcessorBase (TaskConstructor &_constructor) noexcept;

protected:
    void CleanActionComponents () noexcept;

    void DispatchAction (const InputActionHolder *_holder) noexcept;

    FetchValueQuery fetchSubscriptionByGroupId;
    InsertLongTermQuery insertActionComponent;
    RemoveAscendingRangeQuery removeActionComponents;
};

InputProcessorBase::InputProcessorBase (TaskConstructor &_constructor) noexcept
    : fetchSubscriptionByGroupId (FETCH_VALUE_1F (InputSubscriptionComponent, groupId)),
      insertActionComponent (INSERT_LONG_TERM (InputActionComponent)),
      removeActionComponents (REMOVE_ASCENDING_RANGE (InputActionComponent, objectId))
{
}

void InputProcessorBase::CleanActionComponents () noexcept
{
    for (auto cursor = removeActionComponents.Execute (nullptr, nullptr); cursor.ReadConst (); ~cursor)
    {
    }
}

void InputProcessorBase::DispatchAction (const InputActionHolder *_holder) noexcept
{
    auto insertionCursor = insertActionComponent.Execute ();
    for (auto subscriptionCursor = fetchSubscriptionByGroupId.Execute (&_holder->action.group);
         const auto *subscription = static_cast<const InputSubscriptionComponent *> (*subscriptionCursor);
         ++subscriptionCursor)
    {
        auto *action = static_cast<InputActionComponent *> (++insertionCursor);
        action->objectId = subscription->objectId;
        action->action = _holder->action;
    }
}

class FixedInputProcessor final : public TaskExecutorBase<FixedInputProcessor>, public InputProcessorBase
{
public:
    FixedInputProcessor (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    ModifySequenceQuery modifyInputActionHolders;
};

FixedInputProcessor::FixedInputProcessor (TaskConstructor &_constructor) noexcept
    : InputProcessorBase (_constructor),
      modifyInputActionHolders (MODIFY_SEQUENCE (InputActionHolder))
{
    _constructor.DependOn (Checkpoint::ACTION_DISPATCH_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::CUSTOM_ACTION_COMPONENT_INSERT_ALLOWED);
    _constructor.MakeDependencyOf (Checkpoint::ACTION_COMPONENT_READ_ALLOWED);
}

void FixedInputProcessor::Execute () noexcept
{
    CleanActionComponents ();
    for (auto cursor = modifyInputActionHolders.Execute (); auto *holder = static_cast<InputActionHolder *> (*cursor);)
    {
        switch (holder->dispatchType)
        {
        case InputActionDispatchType::NORMAL:
            ++cursor;
            break;

        case InputActionDispatchType::FIXED_INSTANT:
            DispatchAction (holder);
            ~cursor;
            break;

        case InputActionDispatchType::FIXED_PERSISTENT:
            DispatchAction (holder);
            holder->fixedDispatchDone = true;
            ++cursor;
            break;
        }
    }
}

class NormalInputProcessor final : public TaskExecutorBase<NormalInputProcessor>, public InputProcessorBase
{
public:
    NormalInputProcessor (TaskConstructor &_constructor, const FrameInputAccumulator *_inputAccumulator) noexcept;

    void Execute () noexcept;

private:
    void ProcessAccumulatedInput () noexcept;

    void SendAction (const InputAction &_action, InputActionDispatchType _dispatchType) noexcept;

    void DispatchActions () noexcept;

    ModifySequenceQuery modifyInputActionHolders;
    InsertShortTermQuery insertActionHolder;

    EditValueQuery editKeyTriggerByScanCode;
    EditSignalQuery editKeyTriggersOnState;

    const FrameInputAccumulator *inputAccumulator;
};

NormalInputProcessor::NormalInputProcessor (TaskConstructor &_constructor,
                                            const FrameInputAccumulator *_inputAccumulator) noexcept
    : InputProcessorBase (_constructor),
      modifyInputActionHolders (MODIFY_SEQUENCE (InputActionHolder)),
      insertActionHolder (INSERT_SHORT_TERM (InputActionHolder)),

      editKeyTriggerByScanCode (EDIT_VALUE_1F (KeyTrigger, triggerCode)),
      editKeyTriggersOnState (EDIT_SIGNAL (KeyTrigger, triggerType, KeyTriggerType::ON_STATE)),

      inputAccumulator (_inputAccumulator)
{
    _constructor.DependOn (Checkpoint::ACTION_DISPATCH_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::CUSTOM_ACTION_COMPONENT_INSERT_ALLOWED);
    _constructor.MakeDependencyOf (Checkpoint::ACTION_COMPONENT_READ_ALLOWED);
}

void NormalInputProcessor::Execute () noexcept
{
    CleanActionComponents ();
    ProcessAccumulatedInput ();
    DispatchActions ();
}

void NormalInputProcessor::ProcessAccumulatedInput () noexcept
{
    for (const InputEvent &event : inputAccumulator->GetEvents ())
    {
        switch (event.type)
        {
        case InputEventType::KEYBOARD:
        {
            if (event.keyboard.repeat)
            {
                continue;
            }

            for (auto keyTriggerCursor = editKeyTriggerByScanCode.Execute (&event.keyboard.scanCode);
                 auto *keyTrigger = static_cast<KeyTrigger *> (*keyTriggerCursor); ++keyTriggerCursor)
            {
                // We're skipping repeats, so this should never happen.
                EMERGENCE_ASSERT (keyTrigger->currentKeyState != event.keyboard.keyState);
                keyTrigger->currentKeyState = event.keyboard.keyState;

                const bool onStateChanged = keyTrigger->triggerType == KeyTriggerType::ON_STATE_CHANGED;
                const bool onStateAndNotTriggered =
                    !keyTrigger->triggeredThisFrame && keyTrigger->triggerType == KeyTriggerType::ON_STATE;
                const bool canBeTriggered = onStateChanged || onStateAndNotTriggered;

                const bool stateMatches = keyTrigger->triggerTargetState == keyTrigger->currentKeyState;
                const bool qualifiersMatch = keyTrigger->expectedQualifiers == event.keyboard.qualifiersMask;

                if (stateMatches && qualifiersMatch && canBeTriggered)
                {
                    SendAction (keyTrigger->actionToSend, keyTrigger->dispatchType);
                    if (onStateAndNotTriggered)
                    {
                        keyTrigger->triggeredThisFrame = true;
                    }
                }
            }

            break;
        }

        case InputEventType::MOUSE_BUTTON:
        case InputEventType::MOUSE_MOTION:
        case InputEventType::MOUSE_WHEEL:
            // There is no triggers for mouse events right now.
            break;
        }
    }

    for (auto keyTriggerCursor = editKeyTriggersOnState.Execute ();
         auto *keyTrigger = static_cast<KeyTrigger *> (*keyTriggerCursor); ++keyTriggerCursor)
    {
        if (keyTrigger->triggeredThisFrame)
        {
            keyTrigger->triggeredThisFrame = false;
            continue;
        }

        const bool stateMatches = keyTrigger->triggerTargetState == keyTrigger->currentKeyState;
        const bool qualifiersMatch = keyTrigger->expectedQualifiers == inputAccumulator->GetCurrentQualifiersMask ();

        if (stateMatches && qualifiersMatch)
        {
            SendAction (keyTrigger->actionToSend, keyTrigger->dispatchType);
        }
    }
}

void NormalInputProcessor::SendAction (const InputAction &_action, InputActionDispatchType _dispatchType) noexcept
{
    auto cursor = insertActionHolder.Execute ();
    auto *actionHolder = static_cast<InputActionHolder *> (++cursor);
    actionHolder->action = _action;
    actionHolder->dispatchType = _dispatchType;
}

void NormalInputProcessor::DispatchActions () noexcept
{
    for (auto cursor = modifyInputActionHolders.Execute (); auto *holder = static_cast<InputActionHolder *> (*cursor);)
    {
        switch (holder->dispatchType)
        {
        case InputActionDispatchType::NORMAL:
            DispatchAction (holder);
            ~cursor;
            break;

        case InputActionDispatchType::FIXED_INSTANT:
            ++cursor;
            break;

        case InputActionDispatchType::FIXED_PERSISTENT:
            if (holder->fixedDispatchDone)
            {
                ~cursor;
            }
            else
            {
                ++cursor;
            }

            break;
        }
    }
}

static void AddCheckpoints (PipelineBuilder &_builder)
{
    _builder.AddCheckpoint (Checkpoint::ACTION_DISPATCH_STARTED);
    _builder.AddCheckpoint (Checkpoint::CUSTOM_ACTION_COMPONENT_INSERT_ALLOWED);
    _builder.AddCheckpoint (Checkpoint::ACTION_COMPONENT_READ_ALLOWED);
}

void AddToFixedUpdate (PipelineBuilder &_builder) noexcept
{
    using namespace Memory::Literals;
    auto visualGroup = _builder.OpenVisualGroup ("Input");
    AddCheckpoints (_builder);

    _builder.AddTask ("CleanupInputSubscriptionComponentAfterTransformRemoval"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform2dComponentRemovedFixedEvent, InputSubscriptionComponent,
                                objectId)
        .DependOn (Checkpoint::ACTION_DISPATCH_STARTED)
        .MakeDependencyOf ("InputProcessor"_us);

    _builder.AddTask ("InputProcessor"_us).SetExecutor<FixedInputProcessor> ();
}

void AddToNormalUpdate (PipelineBuilder &_builder, const FrameInputAccumulator *_inputAccumulator) noexcept
{
    using namespace Memory::Literals;
    auto visualGroup = _builder.OpenVisualGroup ("Input");
    AddCheckpoints (_builder);

    _builder.AddTask ("CleanupInputSubscriptionComponentAfterTransformRemoval"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform2dComponentRemovedNormalEvent, InputSubscriptionComponent,
                                objectId)
        .DependOn (Checkpoint::ACTION_DISPATCH_STARTED)
        .MakeDependencyOf ("InputProcessor"_us);

    _builder.AddTask ("InputProcessor"_us).SetExecutor<NormalInputProcessor> (_inputAccumulator);
}
} // namespace Emergence::Celerity::Input
