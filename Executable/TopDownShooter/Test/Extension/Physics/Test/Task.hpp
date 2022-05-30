#pragma once

#include <variant>

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <Math/Constants.hpp>
#include <Math/Transform3d.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Physics/CollisionGeometry.hpp>
#include <Physics/RigidBodyComponent.hpp>

namespace Emergence::Physics::Test
{
namespace ConfiguratorTasks
{
struct AddDynamicsMaterial final
{
    Memory::UniqueString id;
    float dynamicFriction = 0.0f;
    float staticFriction = 0.0f;
    bool enableFriction = true;
    float restitution = 0.0f;
    float density = 0.0f;
};

struct RemoveDynamicsMaterial final
{
    Memory::UniqueString id;
    float dynamicFriction = 0.0f;
    float staticFriction = 0.0f;
    bool enableFriction = true;
    float restitution = 0.0f;
    float density = 1.0f;
};

struct AddTransform final
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
    Math::Transform3d transform {Math::Vector3f::ZERO, Math::Quaternion::IDENTITY, Math::Vector3f::ONE};
};

struct RemoveTransform final
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
};

struct AddRigidBody final
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
    RigidBodyType type = RigidBodyType::STATIC;

    float linearDamping = 0.0f;
    float angularDamping = 0.05f;

    bool continuousCollisionDetection = false;
    bool affectedByGravity = true;
    bool manipulatedOutsideOfSimulation = false;
    bool sendContactEvents = false;

    Math::Vector3f linearVelocity = Math::Vector3f::ZERO;
    Math::Vector3f angularVelocity = Math::Vector3f::ZERO;
};

struct RemoveRigidBody final
{
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
};

struct AddCollisionShape final
{
    Celerity::UniqueId shapeId = Celerity::INVALID_UNIQUE_ID;
    Celerity::UniqueId objectId = Celerity::INVALID_UNIQUE_ID;
    Memory::UniqueString materialId;

    CollisionGeometry geometry {.type = CollisionGeometryType::BOX, .boxHalfExtents = {0.5f, 0.5f, 0.5f}};
    Math::Vector3f translation = Math::Vector3f::ONE;
    Math::Quaternion rotation = Math::Quaternion::IDENTITY;

    bool enabled = true;
    bool trigger = false;
    bool visibleToWorldQueries = true;
    uint8_t collisionGroup = 0u;
};

struct RemoveCollisionShape final
{
    Celerity::UniqueId shapeId = Celerity::INVALID_UNIQUE_ID;
};
} // namespace ConfiguratorTasks

using ConfiguratorTask = std::variant<ConfiguratorTasks::AddDynamicsMaterial,
                                      ConfiguratorTasks::RemoveDynamicsMaterial,
                                      ConfiguratorTasks::AddTransform,
                                      ConfiguratorTasks::RemoveTransform,
                                      ConfiguratorTasks::AddRigidBody,
                                      ConfiguratorTasks::RemoveRigidBody,
                                      ConfiguratorTasks::AddCollisionShape,
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
} // namespace ValidatorTasks

using ValidatorTask =
    std::variant<ValidatorTasks::CheckRigidBodyExistence, ValidatorTasks::CheckCollisionShapeExistence>;

struct ValidatorFrame final
{
    uint64_t frameIndex = 0u;
    Container::Vector<ValidatorTask> tasks;
};

constexpr float TEST_FIXED_FRAME_S = 1.0f / 60.0f;

void ExecuteScenario (Container::Vector<ConfiguratorFrame> _configuratorFrames,
                      Container::Vector<ValidatorFrame> _validatorFrames);
} // namespace Emergence::Physics::Test
