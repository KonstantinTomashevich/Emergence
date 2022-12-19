#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct LabelControl final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    Container::Utf8String label;

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
