#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Quaternion.hpp>
#include <Math/Vector3f.hpp>

#include <Memory/UniqueString.hpp>

#include <Physics/CollisionGeometry.hpp>

namespace Emergence::Physics
{
struct CollisionShapeComponent final
{
    EMERGENCE_STATIONARY_DATA_TYPE (CollisionShapeComponent);

    /// \details Must be assigned using PhysicsWorldSingleton::GenerateShapeUID.
    Celerity::UniqueId shapeId = Celerity::INVALID_UNIQUE_ID;

    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;

    Memory::UniqueString materialId;

    /// \invariant Geometry type cannot be changed after initialization!
    CollisionGeometry geometry {.type = CollisionGeometryType::BOX, .boxHalfExtents = {0.5f, 0.5f, 0.5f}};

    Math::Vector3f translation = Math::Vector3f::ONE;

    Math::Quaternion rotation = Math::Quaternion::IDENTITY;

    bool enabled = true;
    bool trigger = false;
    bool visibleToWorldQueries = true;

    /// \invariant < 32u
    uint8_t collisionGroup = 0u;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId shapeId;
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId materialId;
        Emergence::StandardLayout::FieldId geometry;
        Emergence::StandardLayout::FieldId translation;
        Emergence::StandardLayout::FieldId rotation;
        Emergence::StandardLayout::FieldId enabled;
        Emergence::StandardLayout::FieldId trigger;
        Emergence::StandardLayout::FieldId visibleToWorldQueries;
        Emergence::StandardLayout::FieldId collisionGroup;
        Emergence::StandardLayout::FieldId implementationHandle;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
