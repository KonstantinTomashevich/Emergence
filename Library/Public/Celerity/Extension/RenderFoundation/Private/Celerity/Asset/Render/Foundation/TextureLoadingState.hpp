#pragma once

#include <cstdio>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct TextureLoadingState final
{
    TextureLoadingState () noexcept = default;

    TextureLoadingState (const TextureLoadingState &_other) = delete;

    TextureLoadingState (TextureLoadingState &&_other) = delete;

    ~TextureLoadingState () noexcept;

    TextureLoadingState &operator= (const TextureLoadingState &_other) = delete;

    TextureLoadingState &operator= (TextureLoadingState &&_other) = delete;

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
