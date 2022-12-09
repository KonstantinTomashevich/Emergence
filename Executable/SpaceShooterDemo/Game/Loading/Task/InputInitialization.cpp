#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/Input/InputTriggers.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/InputConstant.hpp>

#include <Loading/Model/Events.hpp>
#include <Loading/Task/InputInitialization.hpp>
#include <Loading/Task/LoadingOrchestration.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/ThirdParty/SDL/SDL_scancode.h>
END_MUTING_WARNINGS

namespace InputInitialization
{
class InputInitializer final : public Emergence::Celerity::TaskExecutorBase<InputInitializer>
{
public:
    InputInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::InsertLongTermQuery insertKeyTrigger;
    Emergence::Celerity::FetchSequenceQuery fetchLoadingFinishedEvent;
};

InputInitializer::InputInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : insertKeyTrigger (INSERT_LONG_TERM (Emergence::Celerity::KeyTrigger)),
      fetchLoadingFinishedEvent (FETCH_SEQUENCE (LoadingFinishedEvent))
{
    _constructor.DependOn (LoadingOrchestration::Checkpoint::FINISHED);
}

void InputInitializer::Execute () noexcept
{
    auto eventCursor = fetchLoadingFinishedEvent.Execute ();
    if (!*eventCursor)
    {
        return;
    }

    auto keyTriggerCursor = insertKeyTrigger.Execute ();
    auto *forwardTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    forwardTrigger->actionToSend = {
        InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::MOTION_FACTOR_ACTION,
        std::array<float, Emergence::Celerity::InputAction::MAX_REAL_PARAMETERS> {0.0f, 0.0f, 1.0f}};
    forwardTrigger->triggerCode = SDL_SCANCODE_W;
    forwardTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *backwardTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    backwardTrigger->actionToSend = {
        InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::MOTION_FACTOR_ACTION,
        std::array<float, Emergence::Celerity::InputAction::MAX_REAL_PARAMETERS> {0.0f, 0.0f, -1.0f}};
    backwardTrigger->triggerCode = SDL_SCANCODE_S;
    backwardTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *rightTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    rightTrigger->actionToSend = {
        InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::ROTATION_FACTOR_ACTION,
        std::array<float, Emergence::Celerity::InputAction::MAX_REAL_PARAMETERS> {0.0f, 1.0f, 0.0f}};
    rightTrigger->triggerCode = SDL_SCANCODE_D;
    rightTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *leftTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    leftTrigger->actionToSend = {
        InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::ROTATION_FACTOR_ACTION,
        std::array<float, Emergence::Celerity::InputAction::MAX_REAL_PARAMETERS> {0.0f, -1.0f, 0.0f}};
    leftTrigger->triggerCode = SDL_SCANCODE_A;
    leftTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *fireTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    fireTrigger->actionToSend = {InputConstant::FIGHT_ACTION_GROUP, InputConstant::FIRE_ACTION};
    fireTrigger->triggerCode = SDL_SCANCODE_Q;
    fireTrigger->triggerType = Emergence::Celerity::KeyTriggerType::ON_STATE_CHANGED;
    fireTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_INSTANT;

    auto *slowdownTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    slowdownTrigger->actionToSend = {InputConstant::FIGHT_ACTION_GROUP, InputConstant::SLOWDOWN_ACTION};
    slowdownTrigger->triggerCode = SDL_SCANCODE_E;
    slowdownTrigger->triggerType = Emergence::Celerity::KeyTriggerType::ON_STATE_CHANGED;
    slowdownTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_INSTANT;
}

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("InputInitialization");
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"InputInitializer"}).SetExecutor<InputInitializer> ();
}
} // namespace InputInitialization
