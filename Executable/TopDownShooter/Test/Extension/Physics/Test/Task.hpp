#pragma once

#include <variant>

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <Math/Constants.hpp>
#include <Math/Transform3d.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Physics/CollisionGeometry.hpp>
#include <Physics/Events.hpp>
#include <Physics/RigidBodyComponent.hpp>

namespace Emergence::Physics::Test
{
namespace ConfiguratorTasks
{
struct DynamicsMaterialData
{
    Memory::UniqueString id;
    float dynamicFriction = 0.0f;
    float staticFriction = 0.0f;
    bool enableFriction = true;
    float restitution = 0.0f;
    float density = 1.0f;
};

struct AddDynamicsMaterial final : public DynamicsMaterialData
{
};

struct UpdateDynamicsMaterial final : public DynamicsMaterialData
{
};

struct RemoveDynamicsMaterial final
{
    Memory::UniqueString id;
};

struct TransformData
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
    Math::Transform3d transform {Math::Vector3f::ZERO, Math::Quaternion::IDENTITY, Math::Vector3f::ONE};
};

struct AddTransform final : public TransformData
{
};

struct UpdateTransform final : public TransformData
{
};

struct RemoveTransform final
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
};

struct RigidBodyData
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
    RigidBodyType type = RigidBodyType::STATIC;

    Math::Vector3f linearVelocity = Math::Vector3f::ZERO;
    Math::Vector3f angularVelocity = Math::Vector3f::ZERO;

    float linearDamping = 0.0f;
    float angularDamping = 0.05f;

    bool continuousCollisionDetection = false;
    bool affectedByGravity = true;
    bool manipulatedOutsideOfSimulation = false;

    Math::Vector3f additiveLinearImpulse = Math::Vector3f::ZERO;
    Math::Vector3f additiveAngularImpulse = Math::Vector3f::ZERO;
};

struct AddRigidBody final : public RigidBodyData
{
};

struct UpdateRigidBody final : public RigidBodyData
{
};

struct RemoveRigidBody final
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
};

struct CollisionShapeData
{
    Celerity::UniqueId shapeId = Celerity::INVALID_UNIQUE_ID;
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
    Memory::UniqueString materialId;

    CollisionGeometry geometry {.type = CollisionGeometryType::BOX, .boxHalfExtents = {0.5f, 0.5f, 0.5f}};
    Math::Vector3f translation = Math::Vector3f::ZERO;
    Math::Quaternion rotation = Math::Quaternion::IDENTITY;

    bool enabled = true;
    bool trigger = false;
    bool visibleToWorldQueries = true;
    bool sendContactEvents = false;
    uint8_t collisionGroup = 0u;
};

struct AddCollisionShape final : public CollisionShapeData
{
};

struct UpdateCollisionShape final : public CollisionShapeData
{
};

struct RemoveCollisionShape final
{
    Celerity::UniqueId shapeId = Celerity::INVALID_UNIQUE_ID;
};
} // namespace ConfiguratorTasks

using ConfiguratorTask = std::variant<ConfiguratorTasks::AddDynamicsMaterial,
                                      ConfiguratorTasks::UpdateDynamicsMaterial,
                                      ConfiguratorTasks::RemoveDynamicsMaterial,
                                      ConfiguratorTasks::AddTransform,
                                      ConfiguratorTasks::UpdateTransform,
                                      ConfiguratorTasks::RemoveTransform,
                                      ConfiguratorTasks::AddRigidBody,
                                      ConfiguratorTasks::UpdateRigidBody,
                                      ConfiguratorTasks::RemoveRigidBody,
                                      ConfiguratorTasks::AddCollisionShape,
                                      ConfiguratorTasks::UpdateCollisionShape,
                                      ConfiguratorTasks::RemoveCollisionShape>;

struct ConfiguratorFrame final
{
    uint64_t frameIndex = 0u;
    Container::Vector<ConfiguratorTask> tasks;
};

namespace ValidatorTasks
{
struct CheckRigidBodyExistence final
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
    bool shouldExist = false;
};

struct CheckCollisionShapeExistence final
{
    Celerity::UniqueId shapeId = Celerity::INVALID_UNIQUE_ID;
    bool shouldExist = false;
};

struct CheckObjectTransform final
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
    Math::Transform3d transform {Math::Vector3f::ZERO, Math::Quaternion::IDENTITY, Math::Vector3f::ONE};
};

struct CheckEvents final
{
    Container::Vector<ContactFoundEvent> contactFound;
    Container::Vector<ContactPersistsEvent> contactPersists;
    Container::Vector<ContactLostEvent> contactLost;

    Container::Vector<TriggerEnteredEvent> triggerEntered;
    Container::Vector<TriggerExitedEvent> triggerExited;
};
} // namespace ValidatorTasks

using ValidatorTask = std::variant<ValidatorTasks::CheckRigidBodyExistence,
                                   ValidatorTasks::CheckCollisionShapeExistence,
                                   ValidatorTasks::CheckObjectTransform,
                                   ValidatorTasks::CheckEvents>;

struct ValidatorFrame final
{
    uint64_t frameIndex = 0u;
    Container::Vector<ValidatorTask> tasks;
};

constexpr float TEST_FIXED_FRAME_S = 1.0f / 60.0f;

void ExecuteScenario (Container::Vector<ConfiguratorFrame> _configuratorFrames,
                      Container::Vector<ValidatorFrame> _validatorFrames);
} // namespace Emergence::Physics::Test
