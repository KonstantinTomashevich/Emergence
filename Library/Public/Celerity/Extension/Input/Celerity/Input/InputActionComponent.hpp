#pragma once

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct InputActionComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;

    InputAction action;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId action;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
