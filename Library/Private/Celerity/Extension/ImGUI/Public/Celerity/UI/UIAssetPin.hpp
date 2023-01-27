#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct UIAssetPin final
{
    Memory::UniqueString materialId;

    struct Reflection final
    {
        StandardLayout::FieldId materialId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
