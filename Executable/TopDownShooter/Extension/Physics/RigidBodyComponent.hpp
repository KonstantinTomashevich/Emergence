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
    RigidBodyType type = RigidBodyType::STATIC;

    bool continuousCollisionDetection = false;
    bool affectedByGravity = true;

    Math::Vector3f linearVelocity = Math::Vector3f::ZERO;
    Math::Vector3f angularVelocity = Math::Vector3f::ZERO;

    Math::Vector3f additiveLinearImpulse = Math::Vector3f::ZERO;
    Math::Vector3f additiveAngularImpulse = Math::Vector3f::ZERO;

    float linearDamping = 0.0f;
    float angularDamping = 0.05f;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId type;
        Emergence::StandardLayout::FieldId continuousCollisionDetection;
        Emergence::StandardLayout::FieldId affectedByGravity;
        Emergence::StandardLayout::FieldId linearVelocity;
        Emergence::StandardLayout::FieldId angularVelocity;
        Emergence::StandardLayout::FieldId additiveLinearImpulse;
        Emergence::StandardLayout::FieldId additiveAngularImpulse;
        Emergence::StandardLayout::FieldId linearDamping;
        Emergence::StandardLayout::FieldId angularDamping;
        Emergence::StandardLayout::FieldId implementationHandle;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
