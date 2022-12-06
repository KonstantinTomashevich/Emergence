#pragma once

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Input/Keyboard.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
// TODO: Think about mouse triggers.

enum class KeyTriggerType : uint8_t
{
    ON_STATE = 0u,
    ON_STATE_CHANGED,
};

struct KeyTrigger final
{
    InputAction actionToSend;
    QualifiersMask expectedQualifiers = 0u;
    ScanCode triggerCode = 0u;
    KeyState triggerTargetState = KeyState::DOWN;
    KeyTriggerType triggerType = KeyTriggerType::ON_STATE;
    InputActionDispatchType dispatchType = InputActionDispatchType::NORMAL;

    KeyState currentKeyState = KeyState::UP;
    union
    {
        bool triggeredThisFrame = false;
    };

    struct Reflection final
    {
        StandardLayout::FieldId actionToSend;
        StandardLayout::FieldId expectedQualifiers;
        StandardLayout::FieldId triggerCode;
        StandardLayout::FieldId triggerTargetState;
        StandardLayout::FieldId triggerType;
        StandardLayout::FieldId dispatchType;
        StandardLayout::FieldId currentKeyState;
        StandardLayout::FieldId triggeredThisFrame;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
