#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct UINode final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Memory::UniqueString styleId;
    uint64_t sortIndex = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId parentId;
        StandardLayout::FieldId styleId;
        StandardLayout::FieldId sortIndex;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
