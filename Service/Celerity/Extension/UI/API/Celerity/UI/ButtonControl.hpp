#pragma once

#include <Celerity/Standard/UniqueId.hpp>
#include <Celerity/Input/InputAction.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct ButtonControl final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    Container::Utf8String label;

    uint32_t width = 0u;
    uint32_t height = 0u;

    InputAction onClickAction;
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
