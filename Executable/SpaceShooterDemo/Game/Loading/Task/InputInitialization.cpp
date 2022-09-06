#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/InputConstant.hpp>

#include <Loading/Model/Events.hpp>
#include <Loading/Task/InputInitialization.hpp>
#include <Loading/Task/LoadingOrchestration.hpp>

#include <Input/InputSingleton.hpp>

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
    Emergence::Celerity::ModifySingletonQuery modifyInput;
    Emergence::Celerity::FetchSequenceQuery fetchLoadingFinishedEvent;
};

InputInitializer::InputInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyInput (MODIFY_SINGLETON (InputSingleton)),
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

    auto inputCursor = modifyInput.Execute ();
    auto *input = static_cast<InputSingleton *> (*inputCursor);

    input->keyStateTriggers.EmplaceBack () = {
        {InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::MOTION_FACTOR_ACTION,
         std::array<float, InputAction::MAX_REAL_PARAMETERS> {0.0f, 0.0f, 1.0f}},
        SDL_SCANCODE_W,
        true,
        false};

    input->keyStateTriggers.EmplaceBack () = {
        {InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::MOTION_FACTOR_ACTION,
         std::array<float, InputAction::MAX_REAL_PARAMETERS> {0.0f, 0.0f, -1.0f}},
        SDL_SCANCODE_S,
        true,
        false};

    input->keyStateTriggers.EmplaceBack () = {
        {InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::ROTATION_FACTOR_ACTION,
         std::array<float, InputAction::MAX_REAL_PARAMETERS> {0.0f, 1.0f, 0.0f}},
        SDL_SCANCODE_D,
        true,
        false};

    input->keyStateTriggers.EmplaceBack () = {
        {InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::ROTATION_FACTOR_ACTION,
         std::array<float, InputAction::MAX_REAL_PARAMETERS> {0.0f, -1.0f, 0.0f}},
        SDL_SCANCODE_A,
        true,
        false};

    input->keyStateChangedTriggers.EmplaceBack () = {
        {InputConstant::FIGHT_ACTION_GROUP, InputConstant::FIRE_ACTION}, SDL_SCANCODE_Q, true, false};

    input->keyStateChangedTriggers.EmplaceBack () = {
        {InputConstant::FIGHT_ACTION_GROUP, InputConstant::SLOWDOWN_ACTION}, SDL_SCANCODE_E, true, false};
}

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"InputInitializer"}).SetExecutor<InputInitializer> ();
}
} // namespace InputInitialization
