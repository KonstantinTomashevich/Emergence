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

    Math::Quaternion rotation = Math::Quaternion::IDENTITY;

    Math::Vector3f translation = Math::Vector3f::ZERO;

    /// \invariant Geometry type cannot be changed after initialization!
    CollisionGeometry geometry {.type = CollisionGeometryType::BOX, .boxHalfExtents = {0.5f, 0.5f, 0.5f}};

    Memory::UniqueString materialId;

    bool enabled = true;
    bool trigger = false;
    bool visibleToWorldQueries = true;

    /// \brief Whether collision shape should send events on physical contact during simulation.
    bool sendContactEvents = false;

    /// \invariant < 32u
    uint8_t collisionGroup = 0u;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        StandardLayout::FieldId shapeId;
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId rotation;
        StandardLayout::FieldId translation;
        StandardLayout::FieldId geometry;
        StandardLayout::FieldId materialId;
        StandardLayout::FieldId enabled;
        StandardLayout::FieldId trigger;
        StandardLayout::FieldId visibleToWorldQueries;
        StandardLayout::FieldId sendContactEvents;
        StandardLayout::FieldId collisionGroup;
        StandardLayout::FieldId implementationHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
