#pragma once

#include <Math/Vector3f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents supported types of collision geometry.
enum class CollisionGeometry3dType : std::uint8_t
{
    BOX = 0u,
    SPHERE,
    CAPSULE,
    // TODO: Add convex, triangle soup and height field. Maybe shared?
};

/// \brief Stores collision geometry data along with its type.
struct CollisionGeometry3d final
{
    /// \brief This geometry type.
    CollisionGeometry3dType type;

    union
    {
        /// \details Used if ::type is CollisionGeometry3dType::BOX.
        Math::Vector3f boxHalfExtents;

        /// \details Used if ::type is CollisionGeometry3dType::SPHERE.
        float sphereRadius;

        /// \details Used if ::type is CollisionGeometry3dType::CAPSULE.
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
