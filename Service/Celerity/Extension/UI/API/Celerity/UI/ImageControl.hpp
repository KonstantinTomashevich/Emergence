#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/AxisAlignedBox2d.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct ImageControl final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    UniqueId assetUserId = INVALID_UNIQUE_ID;

    uint32_t width = 0u;
    uint32_t height = 0u;

    Memory::UniqueString textureId;
    Math::AxisAlignedBox2d uv;

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId assetUserId;

        StandardLayout::FieldId width;
        StandardLayout::FieldId height;

        StandardLayout::FieldId textureId;
        StandardLayout::FieldId uv;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
