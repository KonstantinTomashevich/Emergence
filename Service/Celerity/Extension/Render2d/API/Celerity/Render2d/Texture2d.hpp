#pragma once

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Texture2d final
{
    Texture2d () noexcept = default;

    Texture2d (const Texture2d &_other) = delete;

    Texture2d (Texture2d &&_other) = delete;

    ~Texture2d () noexcept;

    Texture2d &operator= (const Texture2d &_other) = delete;

    Texture2d &operator= (Texture2d &&_other) = delete;

    Memory::UniqueString assetId;

    uint64_t nativeHandle = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
