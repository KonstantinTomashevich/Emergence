#pragma once

#include <Celerity/Standard/UniqueId.hpp>
#include <Celerity/Input/InputAction.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents checkbox control.
struct CheckboxControl final
{
    /// \brief Id of an UINode, to which this control is attached.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Text that is displayed near checkbox.
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
        StandardLayout::FieldId checked;

        StandardLayout::FieldId onChangedAction;
        StandardLayout::FieldId onChangedActionDispatch;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
