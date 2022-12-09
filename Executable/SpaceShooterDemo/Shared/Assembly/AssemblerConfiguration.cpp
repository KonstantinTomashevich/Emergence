#include <Assembly/AssemblerConfiguration.hpp>

#include <Celerity/Physics3d/CollisionShape3dComponent.hpp>
#include <Celerity/Physics3d/PhysicsWorld3dSingleton.hpp>
#include <Celerity/Physics3d/RigidBody3dComponent.hpp>
#include <Celerity/Transform/TransformComponent.hpp>

#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/MortalComponent.hpp>
#include <Gameplay/MovementComponent.hpp>
#include <Gameplay/ShooterComponent.hpp>

#include <Render/ParticleEffectComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>

using namespace Emergence::Celerity;

static UniqueId GenerateShapeId (const void *_argument)
{
    return static_cast<const PhysicsWorld3dSingleton *> (_argument)->GenerateShapeId ();
}

static UniqueId GenerateModelId (const void *_argument)
{
    return static_cast<const RenderSceneSingleton *> (_argument)->GenerateModelId ();
}

static UniqueId GenerateEffectId (const void *_argument)
{
    return static_cast<const RenderSceneSingleton *> (_argument)->GenerateEffectId ();
}

static constexpr UniqueId SHAPE_ID_KEY = 0u;

static constexpr UniqueId MODEL_ID_KEY = 1u;

static constexpr UniqueId EFFECT_ID_KEY = 2u;

CustomKeyVector GetAssemblerCustomKeys () noexcept
{
    CustomKeyVector customKeys {GetAssemblerConfigurationAllocationGroup ()};

    CustomKeyDescriptor &shapeIdKey = customKeys.emplace_back ();
    shapeIdKey.singletonProviderType = PhysicsWorld3dSingleton::Reflect ().mapping;
    shapeIdKey.providerFunction = GenerateShapeId;

    CustomKeyDescriptor &modelIdKey = customKeys.emplace_back ();
    modelIdKey.singletonProviderType = RenderSceneSingleton::Reflect ().mapping;
    modelIdKey.providerFunction = GenerateModelId;

    CustomKeyDescriptor &effectIdKey = customKeys.emplace_back ();
    effectIdKey.singletonProviderType = RenderSceneSingleton::Reflect ().mapping;
    effectIdKey.providerFunction = GenerateEffectId;

    return customKeys;
}

TypeBindingVector GetFixedAssemblerTypes () noexcept
{
    TypeBindingVector types {GetAssemblerConfigurationAllocationGroup ()};

    TypeDescriptor &collisionShapeComponent = types.emplace_back ();
    collisionShapeComponent.type = CollisionShape3dComponent::Reflect ().mapping;
    collisionShapeComponent.keys.emplace_back () = {CollisionShape3dComponent::Reflect ().objectId,
                                                    ASSEMBLY_OBJECT_ID_KEY_INDEX};
    collisionShapeComponent.keys.emplace_back () = {CollisionShape3dComponent::Reflect ().shapeId, SHAPE_ID_KEY};

    TypeDescriptor &rigidBodyComponent = types.emplace_back ();
    rigidBodyComponent.type = RigidBody3dComponent::Reflect ().mapping;
    rigidBodyComponent.keys.emplace_back () = {RigidBody3dComponent::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};
    rigidBodyComponent.rotateVector3fs.emplace_back (RigidBody3dComponent::Reflect ().linearVelocity);

    TypeDescriptor &transform3dComponent = types.emplace_back ();
    transform3dComponent.type = Transform3dComponent::Reflect ().mapping;
    transform3dComponent.keys.emplace_back () = {Transform3dComponent::Reflect ().objectId,
                                                 ASSEMBLY_OBJECT_ID_KEY_INDEX};
    transform3dComponent.keys.emplace_back () = {Transform3dComponent::Reflect ().parentObjectId,
                                                 ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &controllableComponent = types.emplace_back ();
    controllableComponent.type = ControllableComponent::Reflect ().mapping;
    controllableComponent.keys.emplace_back () = {ControllableComponent::Reflect ().objectId,
                                                  ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &damageDealerComponent = types.emplace_back ();
    damageDealerComponent.type = DamageDealerComponent::Reflect ().mapping;
    damageDealerComponent.keys.emplace_back () = {DamageDealerComponent::Reflect ().objectId,
                                                  ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &mortalComponent = types.emplace_back ();
    mortalComponent.type = MortalComponent::Reflect ().mapping;
    mortalComponent.keys.emplace_back () = {MortalComponent::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &movementComponent = types.emplace_back ();
    movementComponent.type = MovementComponent::Reflect ().mapping;
    movementComponent.keys.emplace_back () = {MovementComponent::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &shooterComponent = types.emplace_back ();
    shooterComponent.type = ShooterComponent::Reflect ().mapping;
    shooterComponent.keys.emplace_back () = {ShooterComponent::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};
    shooterComponent.keys.emplace_back () = {ShooterComponent::Reflect ().shootingPointObjectId,
                                             ASSEMBLY_OBJECT_ID_KEY_INDEX};

    return types;
}

TypeBindingVector GetNormalAssemblerTypes () noexcept
{
    TypeBindingVector types {GetAssemblerConfigurationAllocationGroup ()};

    TypeDescriptor &staticModelComponent = types.emplace_back ();
    staticModelComponent.type = StaticModelComponent::Reflect ().mapping;
    staticModelComponent.keys.emplace_back () = {StaticModelComponent::Reflect ().objectId,
                                                 ASSEMBLY_OBJECT_ID_KEY_INDEX};
    staticModelComponent.keys.emplace_back () = {StaticModelComponent::Reflect ().modelId, MODEL_ID_KEY};

    TypeDescriptor &particleEffectComponent = types.emplace_back ();
    particleEffectComponent.type = ParticleEffectComponent::Reflect ().mapping;
    particleEffectComponent.keys.emplace_back () = {ParticleEffectComponent::Reflect ().objectId,
                                                    ASSEMBLY_OBJECT_ID_KEY_INDEX};
    particleEffectComponent.keys.emplace_back () = {ParticleEffectComponent::Reflect ().effectId, EFFECT_ID_KEY};

    return types;
}
