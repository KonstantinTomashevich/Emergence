#pragma once

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct FrameBufferTextureUsage final
{
    Memory::UniqueString assetId;
    Memory::UniqueString textureId;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId textureId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
