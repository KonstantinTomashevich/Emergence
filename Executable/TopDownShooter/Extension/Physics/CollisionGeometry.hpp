#pragma once

#include <Math/Vector3f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Physics
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
        Emergence::StandardLayout::FieldId type;
        Emergence::StandardLayout::FieldId boxHalfExtents;
        Emergence::StandardLayout::FieldId sphereRadius;
        Emergence::StandardLayout::FieldId capsuleRadius;
        Emergence::StandardLayout::FieldId capsuleHalfHeight;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
