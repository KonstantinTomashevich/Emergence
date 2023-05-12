#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <Platformer/Control/ControllableComponent.hpp>
#include <Platformer/Control/PlayerControl.hpp>
#include <Platformer/Events.hpp>
#include <Platformer/Input/InputActions.hpp>
#include <Platformer/Team/TeamComponent.hpp>
#include <Platformer/Team/TeamConfigurationSingleton.hpp>

namespace PlayerControl
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"PlayerControl::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"PlayerControl::Finished"};

namespace TaskNames
{
static const Emergence::Memory::UniqueString PLAYER_CONTROL_CLAIMER {"PlayerControl::Claimer"};
static const Emergence::Memory::UniqueString PLAYER_CONTROL_MANAGER {"PlayerControl::Manager"};
} // namespace TaskNames

class PlayerControlClaimer final : public Emergence::Celerity::TaskExecutorBase<PlayerControlClaimer>
{
public:
    PlayerControlClaimer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchTeamConfigurationSingleton;
    Emergence::Celerity::FetchSequenceQuery fetchControllableAddedEvents;
    Emergence::Celerity::FetchValueQuery fetchTeamByObjectId;
    Emergence::Celerity::EditValueQuery editControllableByObjectId;
};

PlayerControlClaimer::PlayerControlClaimer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTeamConfigurationSingleton (FETCH_SINGLETON (TeamConfigurationSingleton)),
      fetchControllableAddedEvents (FETCH_SEQUENCE (ControllableComponentAddedFixedEvent)),
      fetchTeamByObjectId (FETCH_VALUE_1F (TeamComponent, objectId)),
      editControllableByObjectId (EDIT_VALUE_1F (ControllableComponent, objectId))
{
    _constructor.DependOn (Emergence::Celerity::Assembly::Checkpoint::FINISHED);
    _constructor.DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
}

void PlayerControlClaimer::Execute () noexcept
{
    auto teamConfigurationCursor = fetchTeamConfigurationSingleton.Execute ();
    const auto *teamConfiguration = static_cast<const TeamConfigurationSingleton *> (*teamConfigurationCursor);

    for (auto eventCursor = fetchControllableAddedEvents.Execute ();
         const auto *event = static_cast<const ControllableComponentAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        if (auto teamCursor = fetchTeamByObjectId.Execute (&event->objectId);
            const auto *team = static_cast<const TeamComponent *> (*teamCursor))
        {
            if (teamConfiguration->playerTeamId == team->teamId)
            {
                auto controllableCursor = editControllableByObjectId.Execute (&event->objectId);
                auto *controllable = static_cast<ControllableComponent *> (*controllableCursor);

                if (controllable && controllable->control == ControlType::NONE)
                {
                    controllable->control = ControlType::PLAYER;
                }
            }
        }
    }
}

class PlayerControlManager final : public Emergence::Celerity::TaskExecutorBase<PlayerControlManager>
{
public:
    PlayerControlManager (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void AddPlayerControl (Emergence::Celerity::UniqueId _objectId) noexcept;

    void RemovePlayerControl (Emergence::Celerity::UniqueId _objectId) noexcept;

    Emergence::Celerity::FetchSequenceQuery fetchControllableAddedEvents;
    Emergence::Celerity::FetchSequenceQuery fetchControllableChangedEvents;
    Emergence::Celerity::FetchSequenceQuery fetchControllableRemovedEvents;
    Emergence::Celerity::InsertLongTermQuery insertInputSubscription;
    Emergence::Celerity::RemoveValueQuery removeInputSubscriptionById;
};

PlayerControlManager::PlayerControlManager (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchControllableAddedEvents (FETCH_SEQUENCE (ControllableComponentAddedFixedEvent)),
      fetchControllableChangedEvents (FETCH_SEQUENCE (ControllableComponentChangedFixedEvent)),
      fetchControllableRemovedEvents (FETCH_SEQUENCE (ControllableComponentRemovedFixedEvent)),
      insertInputSubscription (INSERT_LONG_TERM (Emergence::Celerity::InputSubscriptionComponent)),
      removeInputSubscriptionById (REMOVE_VALUE_1F (Emergence::Celerity::InputSubscriptionComponent, objectId))
{
    _constructor.DependOn (TaskNames::PLAYER_CONTROL_CLAIMER);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::Input::Checkpoint::ACTION_DISPATCH_STARTED);
}

void PlayerControlManager::Execute () noexcept
{
    for (auto eventCursor = fetchControllableAddedEvents.Execute ();
         const auto *event = static_cast<const ControllableComponentAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        if (event->control == ControlType::PLAYER)
        {
            AddPlayerControl (event->objectId);
        }
    }

    for (auto eventCursor = fetchControllableChangedEvents.Execute ();
         const auto *event = static_cast<const ControllableComponentChangedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        if (event->oldControl == ControlType::PLAYER)
        {
            RemovePlayerControl (event->objectId);
        }

        if (event->newControl == ControlType::PLAYER)
        {
            AddPlayerControl (event->objectId);
        }
    }

    for (auto eventCursor = fetchControllableRemovedEvents.Execute ();
         const auto *event = static_cast<const ControllableComponentRemovedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        if (event->control == ControlType::PLAYER)
        {
            RemovePlayerControl (event->objectId);
        }
    }
}

void PlayerControlManager::AddPlayerControl (Emergence::Celerity::UniqueId _objectId) noexcept
{
    auto cursor = insertInputSubscription.Execute ();
    auto *movementSubscription = static_cast<Emergence::Celerity::InputSubscriptionComponent *> (++cursor);
    movementSubscription->objectId = _objectId;
    movementSubscription->group = PlatformerInputGroups::MOVEMENT_INPUT;
}

void PlayerControlManager::RemovePlayerControl (Emergence::Celerity::UniqueId _objectId) noexcept
{
    for (auto cursor = removeInputSubscriptionById.Execute (&_objectId); cursor.ReadConst ();)
    {
        ~cursor;
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask (TaskNames::PLAYER_CONTROL_CLAIMER).SetExecutor<PlayerControlClaimer> ();
    _pipelineBuilder.AddTask (TaskNames::PLAYER_CONTROL_MANAGER).SetExecutor<PlayerControlManager> ();
}
} // namespace PlayerControl
