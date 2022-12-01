#pragma once

#include <cstdio>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct TextureLoadingState final
{
    EMERGENCE_STATIONARY_DATA_TYPE (TextureLoadingState);

    Memory::UniqueString assetId;
    Memory::Heap allocator {Memory::Profiler::AllocationGroup::Top ()};
    FILE *sourceFile = nullptr;

    uint8_t *data = nullptr;
    uint32_t size = 0u;
    uint32_t read = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId data;
        StandardLayout::FieldId size;
        StandardLayout::FieldId read;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
