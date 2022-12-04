#pragma once

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Input/Keyboard.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
enum class KeyTriggerType : uint8_t
{
    ON_STATE = 0u,
    ON_STATE_CHANGED,
};

struct KeyTrigger final
{
    InputAction actionToSend;

    ScanCode triggerCode = 0u;
    KeyState triggerTargetState = KeyState::DOWN;
    KeyTriggerType triggerType = KeyTriggerType::ON_STATE;
    InputActionDispatchType dispatchType = InputActionDispatchType::NORMAL;

    KeyState currentKeyState = KeyState::UP;

    struct Reflection final
    {
        StandardLayout::FieldId actionToSend;
        StandardLayout::FieldId triggerCode;
        StandardLayout::FieldId triggerTargetState;
        StandardLayout::FieldId triggerType;
        StandardLayout::FieldId dispatchType;
        StandardLayout::FieldId currentKeyState;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
