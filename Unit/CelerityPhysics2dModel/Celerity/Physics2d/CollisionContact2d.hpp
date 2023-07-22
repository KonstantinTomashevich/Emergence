#pragma once

#include <CelerityPhysics2dModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/InplaceVector.hpp>

#include <Math/Vector2f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains information about ongoing collision between shapes that aren't triggers.
/// \details Created only for collision shapes that have CollisionShape2dComponent::maintainCollisionContacts.
///          If both contacting shapes have this flag, two collision contacts will be created for convenience.
struct CelerityPhysics2dModelApi CollisionContact2d final
{
    /// \brief Maximum world reported count of collision points between between shapes.
    static constexpr std::size_t MAX_WORLD_POINTS = 2u;

    /// \brief Unique id of this collision contact.
    UniqueId collisionContactId = INVALID_UNIQUE_ID;

    /// \brief Id of an object to which collision shape with
    ///        CollisionShape2dComponent::maintainCollisionContacts belongs.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Id of the second object engaged in collision.
    UniqueId otherObjectId = INVALID_UNIQUE_ID;

    /// \brief Id of collision shape with CollisionShape2dComponent::maintainCollisionContacts flag.
    UniqueId shapeId = INVALID_UNIQUE_ID;

    /// \brief Id of the second shape engaged in collision.
    UniqueId otherShapeId = INVALID_UNIQUE_ID;

    /// \brief Collision contact normal vector in world coordinates.
    Math::Vector2f normal = Math::Vector2f::UP;

    /// \brief Collision contact points in world coordinates.
    Container::InplaceVector<Math::Vector2f, MAX_WORLD_POINTS> points;

    /// \warning ::normal and ::points are checked using IsNearlyEqual function.
    bool operator== (const CollisionContact2d &_other) const;

    struct CelerityPhysics2dModelApi Reflection final
    {
        StandardLayout::FieldId collisionContactId;
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId otherObjectId;
        StandardLayout::FieldId shapeId;
        StandardLayout::FieldId otherShapeId;
        StandardLayout::FieldId normal;
        StandardLayout::FieldId points;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
