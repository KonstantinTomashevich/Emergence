#pragma once

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Describes type of the data that can be passed to InputControl.
enum class InputControlType : uint8_t
{
    /// \brief Any text can be typed into control.
    TEXT = 0u,

    /// \brief Control only works with integer numbers.
    INT,

    /// \brief Control only works with floating point numbers.
    FLOAT
};

/// \brief Control that gives user ability to input values. Type of values is described by ::type.
struct InputControl final
{
    /// \brief Maximum length of the text that can be typed into InputControlType::TEXT.
    static constexpr size_t MAX_TEXT_LENGTH = 64u;

    /// \brief Id of an UINode, to which this control is attached.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Type of data that can be passed into this input control.
    InputControlType type = InputControlType::TEXT;

    /// \brief Key to LocalizedString that is displayed near the input box.
    Memory::UniqueString labelKey;

    /// \brief Text that is displayed near the input box. Used instead of ::labelKey if present.
    Container::Utf8String label;

    /// \brief Input action that will be sent in InputActionHolder on value change.
    /// \details For InputControlType::INT first discrete parameter of InputAction is set to the new passed value.
    ///          For InputControlType::FLOAT first real parameter of InputAction is set to the new passed value.
    InputAction onChangedAction;

    /// \brief Dispatch type for ::onChangedAction.
    InputActionDispatchType onChangedActionDispatch = InputActionDispatchType::NORMAL;

    union
    {
        /// \brief Current value of InputControlType::TEXT.
        std::array<char, MAX_TEXT_LENGTH> utf8TextValue;

        /// \brief Current value of InputControlType::INT.
        int32_t intValue;

        /// \brief Current value of InputControlType::FLOAT.
        float floatValue;
    };

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId type;
        StandardLayout::FieldId labelKey;
        StandardLayout::FieldId label;

        StandardLayout::FieldId onChangedAction;
        StandardLayout::FieldId onChangedActionDispatch;

        StandardLayout::FieldId utf8TextValue;
        StandardLayout::FieldId intValue;
        StandardLayout::FieldId floatValue;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
