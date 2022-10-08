#include <Celerity/Physics/CollisionShapeComponent.hpp>
#include <Celerity/Physics/RigidBodyComponent.hpp>
#include <Celerity/Transform/TransformComponent.hpp>

#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/MortalComponent.hpp>
#include <Gameplay/MovementComponent.hpp>
#include <Gameplay/ShooterComponent.hpp>

#include <Input/InputListenerComponent.hpp>

#include <Loading/Model/AssetObjectTypeManifest.hpp>

#include <Render/ParticleEffectComponent.hpp>
#include <Render/StaticModelComponent.hpp>

Emergence::Asset::Object::TypeManifest PrepareAssetObjectTypeManifest () noexcept
{
    Emergence::Asset::Object::TypeManifest typeManifest;
    typeManifest.Register (ControllableComponent::Reflect ().mapping, {ControllableComponent::Reflect ().objectId});
    typeManifest.Register (DamageDealerComponent::Reflect ().mapping, {DamageDealerComponent::Reflect ().objectId});

    typeManifest.Register (Emergence::Celerity::CollisionShapeComponent::Reflect ().mapping,
                           {Emergence::Celerity::CollisionShapeComponent::Reflect ().shapeId});

    typeManifest.Register (Emergence::Celerity::RigidBodyComponent::Reflect ().mapping,
                           {Emergence::Celerity::RigidBodyComponent::Reflect ().objectId});

    typeManifest.Register (Emergence::Celerity::Transform3dComponent::Reflect ().mapping,
                           {Emergence::Celerity::Transform3dComponent::Reflect ().objectId});

    typeManifest.Register (InputListenerComponent::Reflect ().mapping, {InputListenerComponent::Reflect ().objectId});

    typeManifest.Register (MortalComponent::Reflect ().mapping, {MortalComponent::Reflect ().objectId});
    typeManifest.Register (MovementComponent::Reflect ().mapping, {MovementComponent::Reflect ().objectId});

    typeManifest.Register (ParticleEffectComponent::Reflect ().mapping, {ParticleEffectComponent::Reflect ().effectId});

    typeManifest.Register (ShooterComponent::Reflect ().mapping, {ShooterComponent::Reflect ().objectId});
    typeManifest.Register (StaticModelComponent::Reflect ().mapping, {StaticModelComponent::Reflect ().modelId});
    return typeManifest;
}
