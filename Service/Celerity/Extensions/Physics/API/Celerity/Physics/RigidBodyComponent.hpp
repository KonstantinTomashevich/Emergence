#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector3f.hpp>

namespace Emergence::Celerity
{
enum class RigidBodyType
{
    STATIC = 0u,
    KINEMATIC,
    DYNAMIC
};

struct RigidBodyComponent final
{
    constexpr static uint8_t LOCK_LINEAR_X = 1u << 0u;
    constexpr static uint8_t LOCK_LINEAR_Y = 1u << 1u;
    constexpr static uint8_t LOCK_LINEAR_Z = 1u << 2u;

    constexpr static uint8_t LOCK_ANGULAR_X = 1u << 3u;
    constexpr static uint8_t LOCK_ANGULAR_Y = 1u << 4u;
    constexpr static uint8_t LOCK_ANGULAR_Z = 1u << 5u;

    EMERGENCE_STATIONARY_DATA_TYPE (RigidBodyComponent);

    UniqueId objectId = INVALID_UNIQUE_ID;

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

    uint8_t lockFlags = 0u;

    // TODO: Currently velocities and impulses are in world coordinates. Is it suitable enough?
    Math::Vector3f linearVelocity = Math::Vector3f::ZERO;
    Math::Vector3f angularVelocity = Math::Vector3f::ZERO;

    Math::Vector3f additiveLinearImpulse = Math::Vector3f::ZERO;
    Math::Vector3f additiveAngularImpulse = Math::Vector3f::ZERO;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId type;
        StandardLayout::FieldId linearDamping;
        StandardLayout::FieldId angularDamping;
        StandardLayout::FieldId continuousCollisionDetection;
        StandardLayout::FieldId affectedByGravity;
        StandardLayout::FieldId manipulatedOutsideOfSimulation;
        StandardLayout::FieldId lockFlags;
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
