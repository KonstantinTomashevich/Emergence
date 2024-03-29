#pragma once

#include <CelerityPhysics2dModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains information about ongoing contact between trigger shape and usual collision shape.
struct CelerityPhysics2dModelApi TriggerContact2d final
{
    /// \brief Unique id of this trigger contact.
    UniqueId triggerContactId = INVALID_UNIQUE_ID;

    /// \brief Id of an object that contains trigger shape.
    UniqueId triggerObjectId = INVALID_UNIQUE_ID;

    /// \brief Id of an object that contains usual collision shape that intersects with trigger shape.
    UniqueId intruderObjectId = INVALID_UNIQUE_ID;

    /// \brief Id of the trigger collision shape.
    UniqueId triggerShapeId = INVALID_UNIQUE_ID;

    /// \brief Id of the usual collision shape that intersects with trigger shape.
    UniqueId intruderShapeId = INVALID_UNIQUE_ID;

    bool operator== (const TriggerContact2d &_other) const = default;

    struct CelerityPhysics2dModelApi Reflection final
    {
        StandardLayout::FieldId triggerContactId;
        StandardLayout::FieldId triggerObjectId;
        StandardLayout::FieldId intruderObjectId;
        StandardLayout::FieldId triggerShapeId;
        StandardLayout::FieldId intruderShapeId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
