#pragma once

#include <CelerityPhysics2dApi.hpp>

#include <API/Common/Shortcuts.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains physical properties of a material needed to execute dynamics simulation.
/// \warning Dynamics material removal causes removal of all associated shapes,
///          because shapes can not exist without material.
struct CelerityPhysics2dApi DynamicsMaterial2d final
{
    /// \brief Readable material id. Should be unique.
    Memory::UniqueString id;

    /// \brief Friction coefficient, valid values are [0.0, 1.0].
    float friction = 0.0f;

    /// \brief Collision restitution coefficient, valid values are [0.0, 1.0].
    float restitution = 0.0f;

    /// \brief Collisions with relative velocity above this value will have restitution applied.
    float restitutionThreshold = 0.0f;

    /// \brief Object weight in mass units (usually kilograms) per square unit (usually square meter).
    float density = 0.0f;

    struct CelerityPhysics2dApi Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId friction;
        StandardLayout::FieldId restitution;
        StandardLayout::FieldId restitutionThreshold;
        StandardLayout::FieldId density;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
