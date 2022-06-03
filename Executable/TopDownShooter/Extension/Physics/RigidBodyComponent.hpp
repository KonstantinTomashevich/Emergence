#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector3f.hpp>

namespace Emergence::Physics
{
enum class RigidBodyType
{
    STATIC = 0u,
    KINEMATIC,
    DYNAMIC
};

struct RigidBodyComponent final
{
    EMERGENCE_STATIONARY_DATA_TYPE (RigidBodyComponent);

    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;

    /// \invariant Can not be changed after body creation.
    RigidBodyType type = RigidBodyType::STATIC;

    float linearDamping = 0.0f;
    float angularDamping = 0.05f;

    bool continuousCollisionDetection = false;
    bool affectedByGravity = true;

    /// \brief Whether rigid body transform, flags, attributes, velocity or impulse
    ///        can be edited outside of physics simulation after object creation.
    /// \details Can be turned on and off at any time. Use it to dynamically enable
    ///          observation when needed and disable when it is no longer needed.
    bool manipulatedOutsideOfSimulation = false;

    Math::Vector3f linearVelocity = Math::Vector3f::ZERO;
    Math::Vector3f angularVelocity = Math::Vector3f::ZERO;

    Math::Vector3f additiveLinearImpulse = Math::Vector3f::ZERO;
    Math::Vector3f additiveAngularImpulse = Math::Vector3f::ZERO;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId type;
        Emergence::StandardLayout::FieldId linearDamping;
        Emergence::StandardLayout::FieldId angularDamping;
        Emergence::StandardLayout::FieldId continuousCollisionDetection;
        Emergence::StandardLayout::FieldId affectedByGravity;
        Emergence::StandardLayout::FieldId manipulatedOutsideOfSimulation;
        Emergence::StandardLayout::FieldId linearVelocity;
        Emergence::StandardLayout::FieldId angularVelocity;
        Emergence::StandardLayout::FieldId additiveLinearImpulse;
        Emergence::StandardLayout::FieldId additiveAngularImpulse;
        Emergence::StandardLayout::FieldId implementationHandle;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
