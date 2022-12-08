#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputActionComponent.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/InputConstant.hpp>
#include <Gameplay/Shooting.hpp>
#include <Gameplay/Slowdown.hpp>
#include <Gameplay/SlowdownSingleton.hpp>

#include <Math/Easing.hpp>
#include <Math/Scalar.hpp>

namespace Slowdown
{
class SlowdownProcessor final : public Emergence::Celerity::TaskExecutorBase<SlowdownProcessor>
{
public:
    SlowdownProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyTime;
    Emergence::Celerity::ModifySingletonQuery modifySlowdown;

    Emergence::Celerity::FetchSignalQuery fetchControlledObject;
    Emergence::Celerity::FetchValueQuery fetchInputActionByObjectId;
};

SlowdownProcessor::SlowdownProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyTime (MODIFY_SINGLETON (Emergence::Celerity::TimeSingleton)),
      modifySlowdown (MODIFY_SINGLETON (SlowdownSingleton)),

      fetchControlledObject (FETCH_SIGNAL (ControllableComponent, controlledByLocalPlayer, true)),
      fetchInputActionByObjectId (FETCH_VALUE_1F (Emergence::Celerity::InputActionComponent, objectId))
{
    _constructor.DependOn (Emergence::Celerity::Input::Checkpoint::ACTION_COMPONENT_READ_ALLOWED);
    _constructor.MakeDependencyOf (Shooting::Checkpoint::STARTED);
}

void SlowdownProcessor::Execute () noexcept
{
    auto timeCursor = modifyTime.Execute ();
    auto *time = static_cast<Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto slowdownCursor = modifySlowdown.Execute ();
    auto *slowdown = static_cast<SlowdownSingleton *> (*slowdownCursor);

    if (slowdown->endTimeNs < time->fixedTimeNs)
    {
        auto controllableCursor = fetchControlledObject.Execute ();
        if (const auto *controllable = static_cast<const ControllableComponent *> (*controllableCursor))
        {
            for (auto inputActionCursor = fetchInputActionByObjectId.Execute (&controllable->objectId);
                 const auto *inputAction =
                     static_cast<const Emergence::Celerity::InputActionComponent *> (*inputActionCursor);
                 ++inputActionCursor)
            {
                if (inputAction->action.id == InputConstant::SLOWDOWN_ACTION)
                {
                    slowdown->endTimeNs = time->fixedTimeNs + slowdown->durationNs;
                    break;
                }
            }
        }
    }

    if (time->fixedTimeNs < slowdown->endTimeNs)
    {
        const uint64_t elapsed = time->fixedTimeNs + slowdown->durationNs - slowdown->endTimeNs;
        const float t = Emergence::Math::Clamp (
            static_cast<float> (elapsed) / static_cast<float> (slowdown->durationNs), 0.0f, 1.0f);

        const float strengthModifier =
            t < 0.5f ? Emergence::Math::EaseOutQuint (t * 2.0f) : Emergence::Math::EaseOutQuint ((1.0f - t) * 2.0f);

        time->timeSpeed = 1.0f - slowdown->strength * strengthModifier;
    }
    else
    {
        time->timeSpeed = 1.0f;
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Slowdown");
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"Slowdown::Processor"})
        .SetExecutor<SlowdownProcessor> ();
}
} // namespace Slowdown
