#pragma once

#include <CelerityPhysics3dModelApi.hpp>

#include <API/Common/Shortcuts.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains physical properties of a material needed to execute dynamics simulation.
/// \warning Dynamics material removal causes removal of all associated shapes,
///          because shapes can not exist without material.
struct CelerityPhysics3dModelApi DynamicsMaterial3d final
{
    DynamicsMaterial3d () noexcept;

    DynamicsMaterial3d (const DynamicsMaterial3d &_other) = delete;

    DynamicsMaterial3d (DynamicsMaterial3d &&_other) noexcept;

    // NOLINTNEXTLINE(performance-trivially-destructible): It's up to implementation to decide whether it is trivial.
    ~DynamicsMaterial3d () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (DynamicsMaterial3d);

    /// \brief Readable material id. Should be unique.
    Memory::UniqueString id;

    /// \brief Dynamic friction coefficient, valid values are [0.0, 1.0].
    float dynamicFriction = 0.0f;

    /// \brief Static friction coefficient, valid values are [0.0, 1.0].
    float staticFriction = 0.0f;

    /// \brief Whether friction is enabled at all.
    bool enableFriction = true;

    /// \brief Collision restitution coefficient, valid values are [0.0, 1.0].
    float restitution = 0.0f;

    /// \brief Object weight in mass units (usually kilograms) per volume unit (usually cubic meter).
    float density = 0.0f;

    /// \brief Pointer to implementation-specific object.
    void *implementationHandle = nullptr;

    struct CelerityPhysics3dModelApi Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId dynamicFriction;
        StandardLayout::FieldId staticFriction;
        StandardLayout::FieldId enableFriction;
        StandardLayout::FieldId restitution;
        StandardLayout::FieldId density;
        StandardLayout::FieldId implementationHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
