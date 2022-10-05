#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#    pragma warning(push)
// We do not care about excessive padding in test tasks. Also, we need to do it before variant declaration.
#    pragma warning(disable : 4324)
#endif

#include <Celerity/Physics/CollisionGeometry.hpp>
#include <Celerity/Physics/Events.hpp>
#include <Celerity/Physics/RigidBodyComponent.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Math/Constants.hpp>
#include <Math/Transform3d.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

namespace Emergence::Celerity::Test
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
    UniqueId objectId = INVALID_UNIQUE_ID;
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
    UniqueId objectId = INVALID_UNIQUE_ID;
};

struct RigidBodyData
{
    UniqueId objectId = INVALID_UNIQUE_ID;
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
    uint8_t lockFlags = 0u;
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

#if defined(_MSC_VER) && !defined(__clang__)
#    pragma warning(pop)
#endif
