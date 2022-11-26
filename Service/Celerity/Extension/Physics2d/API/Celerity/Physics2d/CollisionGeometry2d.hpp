#pragma once

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

/// \brief Stores collision geometry data along with its type.
struct CollisionGeometry2d final
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
        struct
        {
            Math::Vector2f lineStart;

            Math::Vector2f lineEnd;
        };
    };

    struct Reflection final
    {
        StandardLayout::FieldId type;
        StandardLayout::FieldId boxHalfExtents;
        StandardLayout::FieldId circleRadius;
        StandardLayout::FieldId lineStart;
        StandardLayout::FieldId lineEnd;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
