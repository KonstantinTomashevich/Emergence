#pragma once

#include <Math/Vector3f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
enum class CollisionGeometryType
{
    BOX = 0u,
    SPHERE,
    CAPSULE,
    // TODO: Add convex, triangle soup and height field. Maybe shared?
};

struct CollisionGeometry final
{
    CollisionGeometryType type;

    union
    {
        Math::Vector3f boxHalfExtents;

        float sphereRadius;

        struct
        {
            float capsuleRadius;

            float capsuleHalfHeight;
        };
    };

    struct Reflection final
    {
        StandardLayout::FieldId type;
        StandardLayout::FieldId boxHalfExtents;
        StandardLayout::FieldId sphereRadius;
        StandardLayout::FieldId capsuleRadius;
        StandardLayout::FieldId capsuleHalfHeight;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
