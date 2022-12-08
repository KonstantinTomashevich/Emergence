#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>

#include <Gameplay/AlignmentComponent.hpp>
#include <Gameplay/Control.hpp>
#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/InputConstant.hpp>
#include <Gameplay/NonFeatureSpecificComponentCleanup.hpp>
#include <Gameplay/PlayerInfoSingleton.hpp>
#include <Gameplay/Spawn.hpp>

namespace Control
{
using namespace Emergence::Memory::Literals;

class ControlSwitcher final : public Emergence::Celerity::TaskExecutorBase<ControlSwitcher>
{
public:
    ControlSwitcher (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchPlayerInfo;

    Emergence::Celerity::InsertLongTermQuery insertInputSubscription;
    Emergence::Celerity::RemoveValueQuery removeInputSubscriptionByObjectId;

    Emergence::Celerity::FetchValueQuery fetchAlignmentById;
    Emergence::Celerity::RemoveValueQuery removeControllableById;

    Emergence::Celerity::EditAscendingRangeQuery editControllable;
    Emergence::Celerity::EditSignalQuery editControlled;
};

ControlSwitcher::ControlSwitcher (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchPlayerInfo (FETCH_SINGLETON (PlayerInfoSingleton)),

      insertInputSubscription (INSERT_LONG_TERM (Emergence::Celerity::InputSubscriptionComponent)),
      removeInputSubscriptionByObjectId (REMOVE_VALUE_1F (Emergence::Celerity::InputSubscriptionComponent, objectId)),

      fetchAlignmentById (FETCH_VALUE_1F (AlignmentComponent, objectId)),
      removeControllableById (REMOVE_VALUE_1F (ControllableComponent, objectId)),

      editControllable (EDIT_ASCENDING_RANGE (ControllableComponent, objectId)),
      editControlled (EDIT_SIGNAL (ControllableComponent, controlledByLocalPlayer, true))

{
    _constructor.MakeDependencyOf (Emergence::Celerity::Input::Checkpoint::ACTION_DISPATCH_STARTED);
    _constructor.DependOn (NonFeatureSpecificComponentCleanup::Checkpoint::FINISHED);

    // We are consciously adding one-frame delay from object spawn to control takeover.
    // Because otherwise we would need to add delay to all other actions, triggered by
    // input, like shooting a bullet.
    _constructor.MakeDependencyOf (Spawn::Checkpoint::STARTED);
}

void ControlSwitcher::Execute () noexcept
{
    auto playerInfoCursor = fetchPlayerInfo.Execute ();
    const auto *playerInfo = static_cast<const PlayerInfoSingleton *> (*playerInfoCursor);

    const bool playerControlsAnyUnit = [this, playerInfo] ()
    {
        auto controllableCursor = editControlled.Execute ();
        if (auto *controllable = static_cast<ControllableComponent *> (*controllableCursor))
        {
            auto alignmentCursor = fetchAlignmentById.Execute (&controllable->objectId);
            if (const auto *alignment = static_cast<const AlignmentComponent *> (*alignmentCursor))
            {
                if (alignment->playerId == playerInfo->localPlayerUid)
                {
                    return true;
                }

                auto subscriptionCursor = removeInputSubscriptionByObjectId.Execute (&controllable->objectId);
                while (subscriptionCursor.ReadConst ())
                {
                    ~subscriptionCursor;
                }

                controllable->controlledByLocalPlayer = false;
            }
        }

        return false;
    }();

    if (!playerControlsAnyUnit)
    {
        for (auto controllableCursor = editControllable.Execute (nullptr, nullptr);
             auto *controllable = static_cast<ControllableComponent *> (*controllableCursor); ++controllableCursor)
        {
            auto alignmentCursor = fetchAlignmentById.Execute (&controllable->objectId);
            if (const auto *alignment = static_cast<const AlignmentComponent *> (*alignmentCursor))
            {
                if (alignment->playerId == playerInfo->localPlayerUid)
                {
                    controllable->controlledByLocalPlayer = true;
                    auto subscriptionCursor = insertInputSubscription.Execute ();
                    
                    auto *movementSubscription =
                        static_cast<Emergence::Celerity::InputSubscriptionComponent *> (++subscriptionCursor);
                    movementSubscription->objectId = controllable->objectId;
                    movementSubscription->groupId = InputConstant::MOVEMENT_ACTION_GROUP;
                    
                    auto *fightSubscription =
                        static_cast<Emergence::Celerity::InputSubscriptionComponent *> (++subscriptionCursor);
                    fightSubscription->objectId = controllable->objectId;
                    fightSubscription->groupId = InputConstant::FIGHT_ACTION_GROUP;

                    break;
                }
            }
        }
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Control");
    _pipelineBuilder.AddTask ("Control::RemoveControllable"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedEvent, ControllableComponent,
                                objectId)
        .MakeDependencyOf ("Control::Switch"_us);

    _pipelineBuilder.AddTask ("Control::Switch"_us).SetExecutor<ControlSwitcher> ();
}
} // namespace Control
