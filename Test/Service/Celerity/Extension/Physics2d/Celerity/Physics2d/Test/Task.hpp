#pragma once

#include <Celerity/Physics2d/CollisionGeometry2d.hpp>
#include <Celerity/Physics2d/Events.hpp>
#include <Celerity/Physics2d/RigidBody2dComponent.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Math/Constants.hpp>
#include <Math/Transform2d.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

namespace Emergence::Celerity::Test
{
namespace ConfiguratorTasks
{
struct DynamicsMaterialData
{
    Memory::UniqueString id;
    float friction = 0.0f;
    float restitution = 0.0f;
    float restitutionThreshold = 0.5f;
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
    UniqueId objectId = INVALID_UNIQUE_ID;
    Math::Transform2d transform {Math::Vector2f::ZERO, 0.0f, Math::Vector2f::ONE};
};

struct AddTransform final : public TransformData
{
};

struct UpdateTransform final : public TransformData
{
};

struct RemoveTransform final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
};

struct RigidBodyData
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    RigidBody2dType type = RigidBody2dType::STATIC;

    Math::Vector2f linearVelocity = Math::Vector2f::ZERO;
    float angularVelocity = 0.0f;

    float linearDamping = 0.0f;
    float angularDamping = 0.05f;

    bool continuousCollisionDetection = false;
    bool affectedByGravity = true;
    bool manipulatedOutsideOfSimulation = false;

    Math::Vector2f additiveLinearImpulse = Math::Vector2f::ZERO;
    float additiveAngularImpulse = 0.0f;
    bool fixedRotation = false;
};

struct AddRigidBody final : public RigidBodyData
{
};

struct UpdateRigidBody final : public RigidBodyData
{
};

struct RemoveRigidBody final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
};

struct CollisionShapeData
{
    UniqueId shapeId = INVALID_UNIQUE_ID;
    UniqueId objectId = INVALID_UNIQUE_ID;
    Memory::UniqueString materialId;

    CollisionGeometry2d geometry {.type = CollisionGeometry2dType::BOX, .boxHalfExtents = {0.5f, 0.5f}};
    Math::Vector2f translation = Math::Vector2f::ZERO;
    float rotation = 0.0f;

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
    UniqueId shapeId = INVALID_UNIQUE_ID;
};
} // namespace ConfiguratorTasks

using ConfiguratorTask = Container::Variant<ConfiguratorTasks::AddDynamicsMaterial,
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
    UniqueId objectId = INVALID_UNIQUE_ID;
    bool shouldExist = false;
};

struct CheckCollisionShapeExistence final
{
    UniqueId shapeId = INVALID_UNIQUE_ID;
    bool shouldExist = false;
};

struct CheckObjectTransform final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    Math::Transform2d transform {Math::Vector2f::ZERO, 0.0f, Math::Vector2f::ONE};
};

struct CheckEvents final
{
    Container::Vector<Contact2dFoundEvent> contactFound;
    Container::Vector<Contact2dLostEvent> contactLost;

    Container::Vector<Trigger2dEnteredEvent> triggerEntered;
    Container::Vector<Trigger2dExitedEvent> triggerExited;
};
} // namespace ValidatorTasks

using ValidatorTask = Container::Variant<ValidatorTasks::CheckRigidBodyExistence,
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
} // namespace Emergence::Celerity::Test