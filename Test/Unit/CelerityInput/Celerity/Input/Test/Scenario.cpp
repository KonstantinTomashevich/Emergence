#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputActionComponent.hpp>
#include <Celerity/Input/InputActionHolder.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
#include <Celerity/Input/Test/Scenario.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
class TriggerInitializer final : public TaskExecutorBase<TriggerInitializer>
{
public:
    TriggerInitializer (TaskConstructor &_constructor, Container::Vector<KeyTriggerSetup> _keyTriggers) noexcept;

    void Execute () noexcept;

private:
    InsertLongTermQuery insertKeyTrigger;

    Container::Vector<KeyTriggerSetup> keyTriggers;
};

TriggerInitializer::TriggerInitializer (TaskConstructor &_constructor,
                                        Container::Vector<KeyTriggerSetup> _keyTriggers) noexcept
    : TaskExecutorBase (_constructor),

      insertKeyTrigger (INSERT_LONG_TERM (KeyTrigger)),
      keyTriggers (std::move (_keyTriggers))
{
    _constructor.MakeDependencyOf (Input::Checkpoint::ACTION_DISPATCH_STARTED);
}

void TriggerInitializer::Execute () noexcept
{
    auto cursor = insertKeyTrigger.Execute ();
    for (const KeyTriggerSetup &trigger : keyTriggers)
    {
        auto *newTrigger = static_cast<KeyTrigger *> (++cursor);
        newTrigger->actionToSend = trigger.actionToSend;
        newTrigger->expectedQualifiers = trigger.expectedQualifiers;
        newTrigger->triggerCode = trigger.triggerCode;
        newTrigger->triggerTargetState = trigger.triggerTargetState;
        newTrigger->triggerType = trigger.triggerType;
        newTrigger->dispatchType = trigger.dispatchType;
    }

    keyTriggers.clear ();
}

class ExternalActionInserter final : public TaskExecutorBase<ExternalActionInserter>
{
public:
    ExternalActionInserter (TaskConstructor &_constructor,
                            Container::Vector<Container::Vector<ExternalAction>> _actionsPerFrame) noexcept;

    void Execute () noexcept;

private:
    InsertShortTermQuery insertActionHolder;

    std::uint64_t frameIndex = 0u;
    Container::Vector<Container::Vector<ExternalAction>> actionsPerFrame;
};

ExternalActionInserter::ExternalActionInserter (
    TaskConstructor &_constructor, Container::Vector<Container::Vector<ExternalAction>> _actionsPerFrame) noexcept
    : TaskExecutorBase (_constructor),

      insertActionHolder (INSERT_SHORT_TERM (InputActionHolder)),
      actionsPerFrame (std::move (_actionsPerFrame))
{
    _constructor.MakeDependencyOf (Input::Checkpoint::ACTION_DISPATCH_STARTED);
}

void ExternalActionInserter::Execute () noexcept
{
    REQUIRE (frameIndex < actionsPerFrame.size ());
    auto cursor = insertActionHolder.Execute ();

    for (const ExternalAction &externalAction : actionsPerFrame[frameIndex])
    {
        auto *actionHolder = static_cast<InputActionHolder *> (++cursor);
        actionHolder->action = externalAction.action;
        actionHolder->dispatchType = externalAction.dispatchType;
    }

    ++frameIndex;
}

class SubscriptionManager final : public TaskExecutorBase<SubscriptionManager>
{
public:
    SubscriptionManager (TaskConstructor &_constructor,
                         Container::Vector<Container::Vector<SubscriptionInfo>> _subscriptionsToAdd,
                         Container::Vector<Container::Vector<SubscriptionInfo>> _subscriptionsToRemove) noexcept;

    void Execute () noexcept;

private:
    InsertLongTermQuery insertSubscription;
    RemoveValueQuery removeSubscriptionByObjectIdAndGroupId;

    std::uint64_t frameIndex = 0u;
    Container::Vector<Container::Vector<SubscriptionInfo>> subscriptionsToAdd;
    Container::Vector<Container::Vector<SubscriptionInfo>> subscriptionsToRemove;
};

SubscriptionManager::SubscriptionManager (
    TaskConstructor &_constructor,
    Container::Vector<Container::Vector<SubscriptionInfo>> _subscriptionsToAdd,
    Container::Vector<Container::Vector<SubscriptionInfo>> _subscriptionsToRemove) noexcept
    : TaskExecutorBase (_constructor),

      insertSubscription (INSERT_LONG_TERM (InputSubscriptionComponent)),
      removeSubscriptionByObjectIdAndGroupId (REMOVE_VALUE_2F (InputSubscriptionComponent, objectId, group)),

      subscriptionsToAdd (std::move (_subscriptionsToAdd)),
      subscriptionsToRemove (std::move (_subscriptionsToRemove))
{
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Input::Checkpoint::ACTION_DISPATCH_STARTED);
}

void SubscriptionManager::Execute () noexcept
{
    REQUIRE (frameIndex < subscriptionsToAdd.size ());
    REQUIRE (frameIndex < subscriptionsToRemove.size ());

    for (const SubscriptionInfo &info : subscriptionsToAdd[frameIndex])
    {
        auto insertCursor = insertSubscription.Execute ();
        auto *subscription = static_cast<InputSubscriptionComponent *> (++insertCursor);
        subscription->objectId = info.objectId;
        subscription->group = info.groupId;
    }

    for (const SubscriptionInfo &info : subscriptionsToRemove[frameIndex])
    {
        auto removeCursor = removeSubscriptionByObjectIdAndGroupId.Execute (&info);
        REQUIRE (removeCursor.ReadConst ());
        ~removeCursor;
        REQUIRE (!removeCursor.ReadConst ());
    }

    ++frameIndex;
}

class ExpectationValidator final : public TaskExecutorBase<ExpectationValidator>
{
public:
    ExpectationValidator (TaskConstructor &_constructor,
                          Container::Vector<Container::Vector<ActionExpectation>> _expectations) noexcept;

    void Execute () noexcept;

private:
    FetchValueQuery fetchInputActionByObjectId;

    std::uint64_t frameIndex = 0u;
    Container::Vector<Container::Vector<ActionExpectation>> expectations;
};

ExpectationValidator::ExpectationValidator (
    TaskConstructor &_constructor, Container::Vector<Container::Vector<ActionExpectation>> _expectations) noexcept
    : TaskExecutorBase (_constructor),

      fetchInputActionByObjectId (FETCH_VALUE_1F (InputActionComponent, objectId)),
      expectations (std::move (_expectations))
{
    _constructor.DependOn (Input::Checkpoint::ACTION_COMPONENT_READ_ALLOWED);
}

void ExpectationValidator::Execute () noexcept
{
    REQUIRE (frameIndex < expectations.size ());
    for (const ActionExpectation &expectation : expectations[frameIndex])
    {
        Container::Vector<InputAction> actual;
        auto cursor = fetchInputActionByObjectId.Execute (&expectation.objectId);

        while (const auto *inputAction = static_cast<const InputActionComponent *> (*cursor))
        {
            actual.emplace_back (inputAction->action);
            ++cursor;
        }

        // Not the most efficient way to compare unordered vectors, but easy to understand and ok for our small vectors.
        CHECK_EQUAL (expectation.actions.size (), actual.size ());

        for (const InputAction &action : expectation.actions)
        {
            CHECK_EQUAL (std::count (expectation.actions.begin (), expectation.actions.end (), action),
                         std::count (actual.begin (), actual.end (), action));
        }
    }

    ++frameIndex;
}

void ExecuteScenario (const Container::Vector<Update> &_updates,
                      Container::Vector<KeyTriggerSetup> _keyTriggers) noexcept
{
    using namespace Memory::Literals;

    Container::Vector<Container::Vector<SubscriptionInfo>> normalSubscriptionsToAdd;
    Container::Vector<Container::Vector<SubscriptionInfo>> normalSubscriptionsToRemove;

    Container::Vector<Container::Vector<SubscriptionInfo>> fixedSubscriptionsToAdd;
    Container::Vector<Container::Vector<SubscriptionInfo>> fixedSubscriptionsToRemove;

    Container::Vector<Container::Vector<ExternalAction>> normalExternalActions;
    Container::Vector<Container::Vector<ActionExpectation>> normalExpectations;
    Container::Vector<Container::Vector<ActionExpectation>> fixedExpectations;

    for (const Update &update : _updates)
    {
        std::visit (
            [&normalSubscriptionsToAdd, &normalSubscriptionsToRemove, &fixedSubscriptionsToAdd,
             &fixedSubscriptionsToRemove, &normalExternalActions, &normalExpectations,
             &fixedExpectations] (const auto &_update)
            {
                if constexpr (std::is_same_v<NormalUpdate, std::decay_t<decltype (_update)>>)
                {
                    normalSubscriptionsToAdd.emplace_back (_update.subscriptionsToAdd);
                    normalSubscriptionsToRemove.emplace_back (_update.subscriptionsToRemove);
                    normalExternalActions.emplace_back (_update.externalActions);
                    normalExpectations.emplace_back (_update.expectations);
                }
                else if constexpr (std::is_same_v<FixedUpdate, std::decay_t<decltype (_update)>>)
                {
                    fixedSubscriptionsToAdd.emplace_back (_update.subscriptionsToAdd);
                    fixedSubscriptionsToRemove.emplace_back (_update.subscriptionsToRemove);
                    fixedExpectations.emplace_back (_update.expectations);
                }
            },
            update);
    }

    World world {"TestWorld"_us, WorldConfiguration {}};
    PipelineBuilder builder {world.GetRootView ()};
    FrameInputAccumulator inputAccumulator;

    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);

    // Add external checkpoints to which input is connected.
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED);
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::FINISHED);

    builder.AddTask ("ExpectationValidator"_us).SetExecutor<ExpectationValidator> (std::move (fixedExpectations));
    builder.AddTask ("SubscriptionManager"_us)
        .SetExecutor<SubscriptionManager> (std::move (fixedSubscriptionsToAdd), std::move (fixedSubscriptionsToRemove));
    Input::AddToFixedUpdate (builder);
    REQUIRE (builder.End ());

    builder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);

    // Add external checkpoints to which input is connected.
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED);
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::FINISHED);

    builder.AddTask ("ExternalActionInserter"_us)
        .SetExecutor<ExternalActionInserter> (std::move (normalExternalActions));
    builder.AddTask ("ExpectationValidator"_us).SetExecutor<ExpectationValidator> (std::move (normalExpectations));
    builder.AddTask ("SubscriptionManager"_us)
        .SetExecutor<SubscriptionManager> (std::move (normalSubscriptionsToAdd),
                                           std::move (normalSubscriptionsToRemove));
    builder.AddTask ("TriggerInitializer"_us).SetExecutor<TriggerInitializer> (std::move (_keyTriggers));
    Input::AddToNormalUpdate (builder, &inputAccumulator);
    REQUIRE (builder.End ());

    for (const Update &update : _updates)
    {
        std::visit (
            [&world, &inputAccumulator] (const auto &_update)
            {
                if constexpr (std::is_same_v<NormalUpdate, std::decay_t<decltype (_update)>>)
                {
                    for (const InputEvent &event : _update.inputEvents)
                    {
                        inputAccumulator.RecordEvent (event);
                    }

                    inputAccumulator.SetCurrentQualifiersMask (_update.currentQualifiersMask);
                    WorldTestingUtility::RunNormalUpdateOnce (world, 16000000u);
                    inputAccumulator.Clear ();
                }
                else if constexpr (std::is_same_v<FixedUpdate, std::decay_t<decltype (_update)>>)
                {
                    WorldTestingUtility::RunFixedUpdateOnce (world);
                }
            },
            update);
    }
}
} // namespace Emergence::Celerity::Test
