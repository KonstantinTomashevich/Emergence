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

    Memory::UniqueString materialInstanceResourceId;
    Math::Vector2f uv;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId spriteId;
        StandardLayout::FieldId materialInstanceResourceId;
        StandardLayout::FieldId uv;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
