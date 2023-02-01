#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Physics3d/CollisionShape3dComponent.hpp>
#include <Celerity/Physics3d/RigidBody3dComponent.hpp>
#include <Celerity/Transform/TransformComponent.hpp>

#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/MortalComponent.hpp>
#include <Gameplay/MovementComponent.hpp>
#include <Gameplay/ShooterComponent.hpp>

#include <Loading/Model/ResourceObjectTypeManifest.hpp>

#include <Render/ParticleEffectComponent.hpp>
#include <Render/StaticModelComponent.hpp>

Emergence::Resource::Object::TypeManifest PrepareResourceObjectTypeManifest () noexcept
{
    Emergence::Resource::Object::TypeManifest typeManifest;
    typeManifest.InitInjection (Emergence::Celerity::PrototypeComponent::Reflect ().mapping,
                                Emergence::Celerity::PrototypeComponent::Reflect ().objectId);

    typeManifest.Register (ControllableComponent::Reflect ().mapping, {ControllableComponent::Reflect ().objectId});
    typeManifest.Register (DamageDealerComponent::Reflect ().mapping, {DamageDealerComponent::Reflect ().objectId});

    typeManifest.Register (Emergence::Celerity::CollisionShape3dComponent::Reflect ().mapping,
                           {Emergence::Celerity::CollisionShape3dComponent::Reflect ().shapeId});

    typeManifest.Register (Emergence::Celerity::RigidBody3dComponent::Reflect ().mapping,
                           {Emergence::Celerity::RigidBody3dComponent::Reflect ().objectId});

    typeManifest.Register (Emergence::Celerity::Transform3dComponent::Reflect ().mapping,
                           {Emergence::Celerity::Transform3dComponent::Reflect ().objectId});

    typeManifest.Register (MortalComponent::Reflect ().mapping, {MortalComponent::Reflect ().objectId});
    typeManifest.Register (MovementComponent::Reflect ().mapping, {MovementComponent::Reflect ().objectId});

    typeManifest.Register (ParticleEffectComponent::Reflect ().mapping, {ParticleEffectComponent::Reflect ().effectId});

    typeManifest.Register (ShooterComponent::Reflect ().mapping, {ShooterComponent::Reflect ().objectId});
    typeManifest.Register (StaticModelComponent::Reflect ().mapping, {StaticModelComponent::Reflect ().modelId});
    return typeManifest;
}
