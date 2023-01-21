#pragma once

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents clickable button control.
struct ButtonControl final
{
    /// \brief Id of an UINode, to which this control is attached.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Text that is written on this button.
    Container::Utf8String label;

    /// \brief Width of button control.
    uint32_t width = 0u;

    /// \brief Height of button control.
    uint32_t height = 0u;

    /// \brief Input action that will be sent in InputActionHolder on button click.
    InputAction onClickAction;

    /// \brief Dispatch type for ::onClickAction.
    InputActionDispatchType onClickActionDispatch = InputActionDispatchType::NORMAL;

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId width;
        StandardLayout::FieldId height;

        StandardLayout::FieldId onClickAction;
        StandardLayout::FieldId onClickActionDispatch;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
