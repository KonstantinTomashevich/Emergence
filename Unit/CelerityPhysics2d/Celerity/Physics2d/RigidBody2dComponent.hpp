#pragma once

#include <CelerityPhysics2dApi.hpp>

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

namespace Emergence::Celerity
{
/// \brief Enumerates supported rigid body types.
enum class RigidBody2dType
{
    /// \brief Static bodies are not affected by any forces or velocities
    ///        and their transform can not be changed by physics engine.
    STATIC = 0u,

    /// \brief Kinematic bodies are affected by velocities, but not by forces.
    /// \details Collisions do not stop kinematic bodies movement and can not change movement direction.
    KINEMATIC,

    /// \brief Dynamic bodies are affected both by velocities and forces.
    DYNAMIC
};

/// \brief Represents rigid body attached to an object.
struct CelerityPhysics2dApi RigidBody2dComponent final
{
    EMERGENCE_STATIONARY_DATA_TYPE (RigidBody2dComponent);

    /// \brief Id of an object with Transform2dComponent to which rigid body is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Type of this rigid body.
    /// \invariant Can not be changed after body creation.
    RigidBody2dType type = RigidBody2dType::STATIC;

    /// \brief Damping coefficient for linear velocity.
    float linearDamping = 0.0f;

    /// \brief Damping coefficient for angular velocity.
    float angularDamping = 0.05f;

    /// \brief Whether continuous collision detection should be enabled for this object.
    /// \details Only supported for dynamic bodies. Costly from performance point of view.
    bool continuousCollisionDetection = false;

    /// \brief Whether object is affected by gravity.
    /// \details Ignored if body is not dynamic.
    bool affectedByGravity = true;

    /// \brief Whether rigid body transform, flags, attributes, velocity or impulse
    ///        can be edited outside of physics simulation after object creation.
    /// \details Can be turned on and off at any time. Use it to dynamically enable
    ///          observation when needed and disable when it is no longer needed.
    bool manipulatedOutsideOfSimulation = false;

    /// \brief Whether body rotation should be prevented.
    bool fixedRotation = false;

    /// \brief Ignore changes to velocity made by simulation.
    /// \details Used to manipulate dynamic bodies with complex physics, like game characters.
    bool ignoreSimulationVelocityChange = false;

    /// \brief Body linear velocity in world coordinates.
    Math::Vector2f linearVelocity = Math::Vector2f::ZERO;

    /// \brief Body angular velocity in world coordinates.
    float angularVelocity = 0.0f;

    /// \brief Linear impulse in world coordinates, that will be applied to body during next simulation step.
    Math::Vector2f additiveLinearImpulse = Math::Vector2f::ZERO;

    /// \brief Angular impulse in world coordinates, that will be applied to body during next simulation step.
    float additiveAngularImpulse = 0.0f;

    /// \brief Pointer to implementation-specific object.
    void *implementationHandle = nullptr;

    struct CelerityPhysics2dApi Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId type;
        StandardLayout::FieldId linearDamping;
        StandardLayout::FieldId angularDamping;
        StandardLayout::FieldId continuousCollisionDetection;
        StandardLayout::FieldId affectedByGravity;
        StandardLayout::FieldId manipulatedOutsideOfSimulation;
        StandardLayout::FieldId fixedRotation;
        StandardLayout::FieldId ignoreSimulationVelocityChange;
        StandardLayout::FieldId linearVelocity;
        StandardLayout::FieldId angularVelocity;
        StandardLayout::FieldId additiveLinearImpulse;
        StandardLayout::FieldId additiveAngularImpulse;
        StandardLayout::FieldId implementationHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
