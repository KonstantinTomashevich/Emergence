#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Math/Quaternion.hpp>
#include <Math/Vector3f.hpp>

#include <Physics/CollisionGeometry.hpp>

namespace Emergence::Physics
{
struct CollisionShapeComponent final
{
    CollisionShapeComponent () noexcept = default;

    CollisionShapeComponent (const CollisionShapeComponent &_other) = delete;

    CollisionShapeComponent (CollisionShapeComponent &&_other) = delete;

    ~CollisionShapeComponent () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (CollisionShapeComponent);

    CollisionGeometry geometry {.type = CollisionGeometryType::BOX, .boxHalfExtents = {0.5f, 0.5f, 0.5f}};

    Math::Vector3f translation = Math::Vector3f::ONE;

    Math::Quaternion rotation = Math::Quaternion::IDENTITY;

    uint64_t materialId = 0u;

    bool enabled = true;
    bool trigger = false;
    bool visibleToWorldQueries = true;

    /// \invariant < 32u
    uint8_t collisionGroup = 0u;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId geometry;
        Emergence::StandardLayout::FieldId translation;
        Emergence::StandardLayout::FieldId rotation;
        Emergence::StandardLayout::FieldId materialId;
        Emergence::StandardLayout::FieldId enabled;
        Emergence::StandardLayout::FieldId trigger;
        Emergence::StandardLayout::FieldId visibleToWorldQueries;
        Emergence::StandardLayout::FieldId collisionGroup;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
