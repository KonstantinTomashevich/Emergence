#pragma once

#include <queue>

#include <OgreApplicationContext.h>
#include <OgreInput.h>

#include <Input/InputAction.hpp>

#include <SyntaxSugar/InplaceVector.hpp>

struct KeyRequirement final
{
    OgreBites::Keycode key = 0;
    bool justPressed = false;
};

struct KeyboardActionTrigger final
{
    constexpr static const std::size_t MAX_KEY_REQUIREMENTS = 3u;

    InputAction action;
    Emergence::InplaceVector<KeyRequirement, MAX_KEY_REQUIREMENTS> requirements;
};

struct TriggeredAction final
{
    InputAction action;
    uint64_t triggeredAtMs = 0u;
};

class InputAccumulator final : public OgreBites::InputListener
{
public:
    // TODO: Right now we are using OgreBites for simplicity, but Bites application context is a bit
    //       clunky for our purposes. We should migrate to custom alternative of OgreBites::ApplicationContext.
    InputAccumulator (OgreBites::ApplicationContext *_application,
                      const std::vector<KeyboardActionTrigger> &_keyboardTriggers) noexcept;

    InputAccumulator (const InputAccumulator &_other) = delete;

    InputAccumulator (InputAccumulator &&_other) = delete;

    ~InputAccumulator () noexcept;

    [[nodiscard]] bool PopNextAction (InputAction &_output, uint64_t _tillMs) noexcept;

    bool keyPressed (const OgreBites::KeyboardEvent &event) override;

    bool keyReleased (const OgreBites::KeyboardEvent &event) override;

    EMERGENCE_DELETE_ASSIGNMENT (InputAccumulator);

private:
    enum class KeyState
    {
        JUST_PRESSED = 0u,
        PRESSED,
        RELEASED,
    };

    struct KeyRequirementState final
    {
        KeyRequirement requirement;
        KeyState keyCurrentState = KeyState::RELEASED;
    };

    struct KeyboardActionTriggerState final
    {
        InputAction action;
        Emergence::InplaceVector<KeyRequirementState, KeyboardActionTrigger::MAX_KEY_REQUIREMENTS> requirementStates;
    };

    OgreBites::ApplicationContext *application;
    std::queue<TriggeredAction> triggeredActions;
    std::vector<KeyboardActionTriggerState> keyboardTriggers;
};
