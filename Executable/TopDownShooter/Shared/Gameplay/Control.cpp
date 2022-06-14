#include <cassert>

#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/Control.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/InputConstant.hpp>
#include <Gameplay/UnitComponent.hpp>

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
    Emergence::Celerity::EditValueQuery editUnitById;
    Emergence::Celerity::EditSignalQuery editControllableUnits;
    Emergence::Celerity::EditSignalQuery editControlledUnits;
};

ControlSwitcher::ControlSwitcher (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyInput (_constructor.MModifySingleton (InputSingleton)),
      editUnitById (_constructor.MEditValue1F (UnitComponent, objectId)),
      editControllableUnits (_constructor.MEditSignal (UnitComponent, canBeControlledByPlayer, true)),
      editControlledUnits (_constructor.MEditSignal (UnitComponent, controlledByPlayer, true))
{
    _constructor.DependOn (Checkpoint::ASSEMBLY_FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::INPUT_DISPATCH_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::MORTALITY_STARTED);
}

void ControlSwitcher::Execute () noexcept
{
    auto inputCursor = modifyInput.Execute ();
    auto *input = static_cast<InputSingleton *> (*inputCursor);

    const bool playerControlsAnyUnit = [this, input] ()
    {
        auto unitCursor = editControlledUnits.Execute ();
        if (auto *unit = static_cast<UnitComponent *> (*unitCursor))
        {
            if (unit->canBeControlledByPlayer)
            {
                return true;
            }

            unit->controlledByPlayer = false;
            input->UnsubscribeFixed (unit->objectId);
        }

        return false;
    }();

    if (!playerControlsAnyUnit)
    {
        auto unitCursor = editControllableUnits.Execute ();
        if (auto *unit = static_cast<UnitComponent *> (*unitCursor))
        {
            unit->controlledByPlayer = true;
            input->fixedSubscriptions.EmplaceBack () = {InputConstant::MOVEMENT_ACTION_GROUP, unit->objectId};
            input->fixedSubscriptions.EmplaceBack () = {InputConstant::FIGHT_ACTION_GROUP, unit->objectId};
        }
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Control::Switch"_us).SetExecutor<ControlSwitcher> ();
}
} // namespace Control
