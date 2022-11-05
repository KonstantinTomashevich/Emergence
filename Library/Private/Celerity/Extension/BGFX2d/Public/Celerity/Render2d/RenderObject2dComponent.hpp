#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/AxisAlignedBox2d.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct RenderObject2dComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    Math::AxisAlignedBox2d global {Math::Vector2f::ZERO, Math::Vector2f::ZERO};
    Math::AxisAlignedBox2d local {Math::Vector2f::ZERO, Math::Vector2f::ZERO};
    bool globalDirty = false;
    bool localDirty = false;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId global;
        StandardLayout::FieldId local;
        StandardLayout::FieldId globalDirty;
        StandardLayout::FieldId localDirty;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct LocalBounds2dComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId renderObjectId = INVALID_UNIQUE_ID;

    Math::AxisAlignedBox2d bounds {Math::Vector2f::ZERO, Math::Vector2f::ZERO};
    bool dirty = false;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId renderObjectId;
        StandardLayout::FieldId bounds;
        StandardLayout::FieldId dirty;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
