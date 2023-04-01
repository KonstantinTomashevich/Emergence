#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Physics2d/CollisionGeometry2d.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Quaternion.hpp>
#include <Math/Vector2f.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Celerity
{
/// \brief Represents collision shape, attached to an object.
/// \details There could be multiple shapes, attached to the same object.
struct CollisionShape2dComponent final
{
    EMERGENCE_STATIONARY_DATA_TYPE (CollisionShape2dComponent);

    /// \brief Unique id of this shape component.
    /// \details Because there could be multiple shapes per object, we need another unique identification system.
    /// \invariant Must be assigned using PhysicsWorld2dSingleton::GenerateShapeId.
    UniqueId shapeId = INVALID_UNIQUE_ID;

    /// \brief Id of an object to which this shape is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Shape additional translation, local to object transform.
    Math::Vector2f translation = Math::Vector2f::ZERO;

    /// \brief Shape additional rotation, local to object transform.
    float rotation = 0.0f;

    /// \brief Shape geometry. Can be resized any time in fixed pipeline.
    /// \invariant Geometry type cannot be changed after initialization!
    CollisionGeometry2d geometry {.type = CollisionGeometry2dType::BOX, .boxHalfExtents = {0.5f, 0.5f}};

    /// \brief Shape physical material id.
    /// \see DynamicsMaterial2d
    Memory::UniqueString materialId;

    /// \brief Whether this shape is a trigger shape.
    /// \details Trigger shapes do not collide, but send special enter/exit events.
    bool trigger = false;

    /// \brief Whether this shape is visible to world queries.
    bool visibleToWorldQueries = true;

    /// \brief Whether collision shape should maintain list of CollisionContact2d's.
    /// \details Ignored if ::trigger.
    /// \invariant Should not be changed after component creation: changing it is not yet supported.
    bool maintainCollisionContacts = false;

    /// \brief Shape collision groups are used to filter out unneeded collisions.
    /// \invariant < 32u
    uint8_t collisionGroup = 0u;

    /// \brief Pointer to implementation-specific object.
    void *implementationHandle = nullptr;

    struct Reflection final
    {
        StandardLayout::FieldId shapeId;
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId rotation;
        StandardLayout::FieldId translation;
        StandardLayout::FieldId geometry;
        StandardLayout::FieldId materialId;
        StandardLayout::FieldId trigger;
        StandardLayout::FieldId visibleToWorldQueries;
        StandardLayout::FieldId maintainCollisionContacts;
        StandardLayout::FieldId collisionGroup;
        StandardLayout::FieldId implementationHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
