#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>

#include <Gameplay/AlignmentComponent.hpp>
#include <Gameplay/Control.hpp>
#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/InputConstant.hpp>
#include <Gameplay/PlayerInfoSingleton.hpp>

#include <Input/InputSingleton.hpp>

#include <Shared/Checkpoint.hpp>

namespace Control
{
using namespace Emergence::Memory::Literals;

class ControlSwitcher final : public Emergence::Celerity::TaskExecutorBase<ControlSwitcher>
{
public:
    ControlSwitcher (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyInput;
    Emergence::Celerity::FetchSingletonQuery fetchPlayerInfo;

    Emergence::Celerity::FetchValueQuery fetchAlignmentById;
    Emergence::Celerity::RemoveValueQuery removeControllableById;

    Emergence::Celerity::EditAscendingRangeQuery editControllable;
    Emergence::Celerity::EditSignalQuery editControlled;
};

ControlSwitcher::ControlSwitcher (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyInput (MODIFY_SINGLETON (InputSingleton)),
      fetchPlayerInfo (FETCH_SINGLETON (PlayerInfoSingleton)),

      fetchAlignmentById (FETCH_VALUE_1F (AlignmentComponent, objectId)),
      removeControllableById (REMOVE_VALUE_1F (ControllableComponent, objectId)),

      editControllable (EDIT_ASCENDING_RANGE (ControllableComponent, objectId)),
      editControlled (EDIT_SIGNAL (ControllableComponent, controlledByLocalPlayer, true))

{
    _constructor.MakeDependencyOf (Checkpoint::INPUT_DISPATCH_STARTED);

    // We are consciously adding one-frame delay from object spawn to control takeover.
    // Because otherwise we would need to add delay to all other actions, triggered by
    // input, like shooting a bullet.
    _constructor.MakeDependencyOf (Checkpoint::SPAWN_STARTED);
}

void ControlSwitcher::Execute () noexcept
{
    auto inputCursor = modifyInput.Execute ();
    auto *input = static_cast<InputSingleton *> (*inputCursor);

    auto playerInfoCursor = fetchPlayerInfo.Execute ();
    const auto *playerInfo = static_cast<const PlayerInfoSingleton *> (*playerInfoCursor);

    const bool playerControlsAnyUnit = [this, input, playerInfo] ()
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

                input->UnsubscribeFixed (controllable->objectId);
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
                    input->fixedSubscriptions.EmplaceBack () = {InputConstant::MOVEMENT_ACTION_GROUP,
                                                                controllable->objectId};
                    input->fixedSubscriptions.EmplaceBack () = {InputConstant::FIGHT_ACTION_GROUP,
                                                                controllable->objectId};

                    break;
                }
            }
        }
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Control::RemoveControllable"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedEvent, ControllableComponent,
                                objectId)
        .MakeDependencyOf ("Control::Switch"_us);

    _pipelineBuilder.AddTask ("Control::Switch"_us).SetExecutor<ControlSwitcher> ();
}
} // namespace Control
