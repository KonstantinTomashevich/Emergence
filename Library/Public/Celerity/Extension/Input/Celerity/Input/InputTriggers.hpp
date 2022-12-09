#pragma once

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Input/Keyboard.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
// TODO: Think about mouse triggers.

/// \brief Describes how trigger is activated.
enum class KeyTriggerType : uint8_t
{
    /// \brief Trigger is active when selected key has selected state, but cannot be activated more than once per frame.
    ON_STATE = 0u,

    /// \brief Trigger is active when selected key state has changed and became equal to selected state.
    /// \details Can be activated multiple times per frame if user mashes the button.
    ON_STATE_CHANGED,
};

/// \brief Automatically adds action to dispatch queue if given keyboard conditions are met.
struct KeyTrigger final
{
    /// \brief Action that will be sent when conditions are met.
    InputAction actionToSend;

    /// \brief Required keyboard qualifiers state for the trigger to be active.
    QualifiersMask expectedQualifiers = 0u;

    /// \brief Selected key layout-independent code.
    ScanCode triggerCode = 0u;

    /// \brief Selected key state.
    KeyState triggerTargetState = KeyState::DOWN;

    /// \brief Describes how trigger activation works.
    KeyTriggerType triggerType = KeyTriggerType::ON_STATE;

    /// \brief Dispatch type for ::actionToSend.
    InputActionDispatchType dispatchType = InputActionDispatchType::NORMAL;

    /// \brief Current observed state of the key, used for internal logic. Can be left at default value.
    KeyState currentKeyState = KeyState::UP;

    union
    {
        /// \brief Internal flag for KeyTriggerType::ON_STATE that is used to avoid multiple activations per frame.
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
