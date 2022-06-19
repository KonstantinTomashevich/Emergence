#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/Control.hpp>
#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/InputConstant.hpp>

#include <Input/InputSingleton.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>

namespace Control
{
using namespace Emergence::Memory::Literals;

class ControlSwitcher final : public Emergence::Celerity::TaskExecutorBase<ControlSwitcher>
{
public:
    ControlSwitcher (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedEvents;
    Emergence::Celerity::ModifySingletonQuery modifyInput;

    Emergence::Celerity::RemoveValueQuery removeControllableById;
    Emergence::Celerity::EditAscendingRangeQuery editControllable;
    Emergence::Celerity::EditSignalQuery editControlled;
};

ControlSwitcher::ControlSwitcher (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTransformRemovedEvents (FETCH_SEQUENCE (Emergence::Transform::Transform3dComponentRemovedFixedEvent)),
      modifyInput (MODIFY_SINGLETON (InputSingleton)),

      removeControllableById (REMOVE_VALUE_1F (ControllableComponent, objectId)),
      editControllable (EDIT_ASCENDING_RANGE (ControllableComponent, objectId)),
      editControlled (EDIT_SIGNAL (ControllableComponent, controlledByPlayer, true))

{
    _constructor.MakeDependencyOf (Checkpoint::INPUT_DISPATCH_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::MORTALITY_STARTED);

    // We are consciously adding one-frame delay from object spawn to control takeover.
    // Because otherwise we would need to add delay to all other actions, triggered by
    // input, like shooting a bullet.
    _constructor.MakeDependencyOf (Checkpoint::ASSEMBLY_STARTED);
}

void ControlSwitcher::Execute () noexcept
{
    auto inputCursor = modifyInput.Execute ();
    auto *input = static_cast<InputSingleton *> (*inputCursor);

    for (auto eventCursor = fetchTransformRemovedEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto controllableCursor = removeControllableById.Execute (&event->objectId);
        if (controllableCursor.ReadConst ())
        {
            ~controllableCursor;
        }
    }

    const bool playerControlsAnyUnit = [this] ()
    {
        auto controllableCursor = editControlled.Execute ();
        return *controllableCursor;
    }();

    if (!playerControlsAnyUnit)
    {
        auto controllableCursor = editControllable.Execute (nullptr, nullptr);
        if (auto *controllable = static_cast<ControllableComponent *> (*controllableCursor))
        {
            controllable->controlledByPlayer = true;
            input->fixedSubscriptions.EmplaceBack () = {InputConstant::MOVEMENT_ACTION_GROUP, controllable->objectId};
            input->fixedSubscriptions.EmplaceBack () = {InputConstant::FIGHT_ACTION_GROUP, controllable->objectId};
        }
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Control::Switch"_us).SetExecutor<ControlSwitcher> ();
}
} // namespace Control
