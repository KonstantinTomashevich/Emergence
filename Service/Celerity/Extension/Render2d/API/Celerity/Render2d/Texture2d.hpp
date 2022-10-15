#pragma once

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Texture2d final
{
    Memory::UniqueString resourceId;

    uint64_t nativeHandle = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId resourceId;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
