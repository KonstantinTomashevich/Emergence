#pragma once

#include <CelerityUIModelApi.hpp>

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents clickable button control.
struct CelerityUIModelApi ButtonControl final
{
    /// \brief Id of an UINode, to which this control is attached.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Key to LocalizedString that is written on this button.
    Memory::UniqueString labelKey;

    /// \brief Text that is written on this button. Used instead of ::labelKey if present.
    Container::Utf8String label;

    /// \brief Width of button control.
    std::uint32_t width = 0u;

    /// \brief Height of button control.
    std::uint32_t height = 0u;

    /// \brief Input action that will be sent in InputActionHolder on button click.
    InputAction onClickAction;

    /// \brief Dispatch type for ::onClickAction.
    InputActionDispatchType onClickActionDispatch = InputActionDispatchType::NORMAL;

    struct CelerityUIModelApi Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId labelKey;
        StandardLayout::FieldId label;
        StandardLayout::FieldId width;
        StandardLayout::FieldId height;

        StandardLayout::FieldId onClickAction;
        StandardLayout::FieldId onClickActionDispatch;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
