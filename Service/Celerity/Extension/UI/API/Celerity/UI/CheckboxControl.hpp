#pragma once

#include <Celerity/Standard/UniqueId.hpp>
#include <Celerity/Input/InputAction.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct CheckboxControl final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    Container::Utf8String label;
    bool checked = false;

    InputAction onChangedAction;
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
