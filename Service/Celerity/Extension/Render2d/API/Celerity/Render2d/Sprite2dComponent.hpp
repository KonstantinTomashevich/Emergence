#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/AxisAlignedBox2d.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Sprite2dComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId spriteId = INVALID_UNIQUE_ID;
    UniqueId assetUserId = INVALID_UNIQUE_ID;

    Memory::UniqueString materialInstanceId;
    Math::AxisAlignedBox2d uv;
    Math::Vector2f halfSize;
    uint16_t layer = 0u;
    std::uint64_t visibilityMask = ~0u;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId spriteId;
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId materialInstanceId;
        StandardLayout::FieldId uv;
        StandardLayout::FieldId halfSize;
        StandardLayout::FieldId layer;
        StandardLayout::FieldId visibilityMask;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
