#include <Assembly/AssemblerConfiguration.hpp>

#include <Celerity/Physics/CollisionShapeComponent.hpp>
#include <Celerity/Physics/PhysicsWorldSingleton.hpp>
#include <Celerity/Physics/RigidBodyComponent.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>

#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/MortalComponent.hpp>
#include <Gameplay/MovementComponent.hpp>
#include <Gameplay/ShooterComponent.hpp>

#include <Input/InputListenerComponent.hpp>

#include <Render/ParticleEffectComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>

using namespace Emergence::Celerity;

static UniqueId GenerateShapeId (const void *_argument)
{
    return static_cast<const PhysicsWorldSingleton *> (_argument)->GenerateShapeId ();
}

static AssemblerConfiguration GetFixedAssemblerConfiguration () noexcept
{
    AssemblerConfiguration configuration;

    const UniqueId shapeIdKeyIndex = configuration.customKeys.size ();
    CustomKeyDescriptor &shapeIdKey = configuration.customKeys.emplace_back ();
    shapeIdKey.singletonProviderType = PhysicsWorldSingleton::Reflect ().mapping;
    shapeIdKey.providerFunction = GenerateShapeId;

    TypeDescriptor &collisionShapeComponent = configuration.types.emplace_back ();
    collisionShapeComponent.type = CollisionShapeComponent::Reflect ().mapping;
    collisionShapeComponent.keys.emplace_back () = {CollisionShapeComponent::Reflect ().objectId,
                                                    ASSEMBLY_OBJECT_ID_KEY_INDEX};
    collisionShapeComponent.keys.emplace_back () = {CollisionShapeComponent::Reflect ().shapeId, shapeIdKeyIndex};

    TypeDescriptor &rigidBodyComponent = configuration.types.emplace_back ();
    rigidBodyComponent.type = RigidBodyComponent::Reflect ().mapping;
    rigidBodyComponent.keys.emplace_back () = {RigidBodyComponent::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};
    rigidBodyComponent.rotateVector3fs.emplace_back (RigidBodyComponent::Reflect ().linearVelocity);

    TypeDescriptor &transform3dComponent = configuration.types.emplace_back ();
    transform3dComponent.type = Transform3dComponent::Reflect ().mapping;
    transform3dComponent.keys.emplace_back () = {Transform3dComponent::Reflect ().objectId,
                                                 ASSEMBLY_OBJECT_ID_KEY_INDEX};
    transform3dComponent.keys.emplace_back () = {Transform3dComponent::Reflect ().parentObjectId,
                                                 ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &controllableComponent = configuration.types.emplace_back ();
    controllableComponent.type = ControllableComponent::Reflect ().mapping;
    controllableComponent.keys.emplace_back () = {ControllableComponent::Reflect ().objectId,
                                                  ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &damageDealerComponent = configuration.types.emplace_back ();
    damageDealerComponent.type = DamageDealerComponent::Reflect ().mapping;
    damageDealerComponent.keys.emplace_back () = {DamageDealerComponent::Reflect ().objectId,
                                                  ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &mortalComponent = configuration.types.emplace_back ();
    mortalComponent.type = MortalComponent::Reflect ().mapping;
    mortalComponent.keys.emplace_back () = {MortalComponent::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &movementComponent = configuration.types.emplace_back ();
    movementComponent.type = MovementComponent::Reflect ().mapping;
    movementComponent.keys.emplace_back () = {MovementComponent::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &shooterComponent = configuration.types.emplace_back ();
    shooterComponent.type = ShooterComponent::Reflect ().mapping;
    shooterComponent.keys.emplace_back () = {ShooterComponent::Reflect ().objectId, ASSEMBLY_OBJECT_ID_KEY_INDEX};
    shooterComponent.keys.emplace_back () = {ShooterComponent::Reflect ().shootingPointObjectId,
                                             ASSEMBLY_OBJECT_ID_KEY_INDEX};

    TypeDescriptor &inputListenerComponent = configuration.types.emplace_back ();
    inputListenerComponent.type = InputListenerComponent::Reflect ().mapping;
    inputListenerComponent.keys.emplace_back () = {InputListenerComponent::Reflect ().objectId,
                                                   ASSEMBLY_OBJECT_ID_KEY_INDEX};

    return configuration;
}

static UniqueId GenerateModelId (const void *_argument)
{
    return static_cast<const RenderSceneSingleton *> (_argument)->GenerateModelId ();
}

static UniqueId GenerateEffectId (const void *_argument)
{
    return static_cast<const RenderSceneSingleton *> (_argument)->GenerateEffectId ();
}

static AssemblerConfiguration GetNormalAssemblerConfiguration () noexcept
{
    AssemblerConfiguration configuration;

    const UniqueId modelIdKeyIndex = configuration.customKeys.size ();
    CustomKeyDescriptor &modelIdKey = configuration.customKeys.emplace_back ();
    modelIdKey.singletonProviderType = RenderSceneSingleton::Reflect ().mapping;
    modelIdKey.providerFunction = GenerateModelId;

    const UniqueId effectIdKeyIndex = configuration.customKeys.size ();
    CustomKeyDescriptor &effectIdKey = configuration.customKeys.emplace_back ();
    effectIdKey.singletonProviderType = RenderSceneSingleton::Reflect ().mapping;
    effectIdKey.providerFunction = GenerateEffectId;

    TypeDescriptor &staticModelComponent = configuration.types.emplace_back ();
    staticModelComponent.type = StaticModelComponent::Reflect ().mapping;
    staticModelComponent.keys.emplace_back () = {StaticModelComponent::Reflect ().objectId,
                                                 ASSEMBLY_OBJECT_ID_KEY_INDEX};
    staticModelComponent.keys.emplace_back () = {StaticModelComponent::Reflect ().modelId, modelIdKeyIndex};

    TypeDescriptor &particleEffectComponent = configuration.types.emplace_back ();
    particleEffectComponent.type = ParticleEffectComponent::Reflect ().mapping;
    particleEffectComponent.keys.emplace_back () = {ParticleEffectComponent::Reflect ().objectId,
                                                    ASSEMBLY_OBJECT_ID_KEY_INDEX};
    particleEffectComponent.keys.emplace_back () = {ParticleEffectComponent::Reflect ().effectId, effectIdKeyIndex};

    return configuration;
}
