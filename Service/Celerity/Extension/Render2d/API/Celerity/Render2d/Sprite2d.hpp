#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Sprite2d final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId spriteId = INVALID_UNIQUE_ID;
    UniqueId assetUserId = INVALID_UNIQUE_ID;

    Memory::UniqueString materialInstanceAssetId;
    Math::Vector2f uv;
    std::uint16_t layer = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId spriteId;
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId materialInstanceAssetId;
        StandardLayout::FieldId uv;
        StandardLayout::FieldId layer;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
