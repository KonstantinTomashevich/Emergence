#pragma once

#include <CelerityPhysics2dModelApi.hpp>

#include <Math/Vector2f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents supported types of collision geometry.
enum class CollisionGeometry2dType : uint8_t
{
    BOX = 0u,
    CIRCLE,
    LINE,
    // TODO: Add polygons, chains and one-sided lines.
};

/// \brief Two dimensional line that can be used as CollisionGeometry2d.
struct CelerityPhysics2dModelApi Line2d final
{
    /// \brief Line start point.
    Math::Vector2f start;

    /// \brief Line end point.
    Math::Vector2f end;

    struct CelerityPhysics2dModelApi Reflection final
    {
        StandardLayout::FieldId start;
        StandardLayout::FieldId end;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Stores collision geometry data along with its type.
struct CelerityPhysics2dModelApi CollisionGeometry2d final
{
    /// \brief This geometry type.
    CollisionGeometry2dType type;

    union
    {
        /// \details Used if ::type is CollisionGeometry2dType::BOX.
        Math::Vector2f boxHalfExtents;

        /// \details Used if ::type is CollisionGeometry2dType::CIRCLE.
        float circleRadius;

        /// \details Used if ::type is CollisionGeometry2dType::LINE.
        Line2d line;
    };

    struct CelerityPhysics2dModelApi Reflection final
    {
        StandardLayout::FieldId type;
        StandardLayout::FieldId boxHalfExtents;
        StandardLayout::FieldId circleRadius;
        StandardLayout::FieldId line;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
