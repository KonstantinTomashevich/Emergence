#pragma once

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents checkbox control.
struct CheckboxControl final
{
    /// \brief Id of an UINode, to which this control is attached.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Key to LocalizedString that is displayed near the checkbox.
    Memory::UniqueString labelKey;

    /// \brief Text that is displayed near the checkbox. Used instead of ::labelKey if present.
    Container::Utf8String label;

    /// \brief Whether checkbox is checked right now. Automatically updated by UI.
    bool checked = false;

    /// \brief Input action that will be sent in InputActionHolder on checkbox state change.
    InputAction onChangedAction;

    /// \brief Dispatch type for ::onChangedAction.
    InputActionDispatchType onChangedActionDispatch = InputActionDispatchType::NORMAL;

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId labelKey;
        StandardLayout::FieldId checked;

        StandardLayout::FieldId onChangedAction;
        StandardLayout::FieldId onChangedActionDispatch;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
