#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>
#include <Celerity/Physics2d/RigidBody2dComponent.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/UI/ButtonControl.hpp>
#include <Celerity/UI/CheckboxControl.hpp>
#include <Celerity/UI/ContainerControl.hpp>
#include <Celerity/UI/ImageControl.hpp>
#include <Celerity/UI/InputControl.hpp>
#include <Celerity/UI/LabelControl.hpp>
#include <Celerity/UI/UINode.hpp>
#include <Celerity/UI/UIStyle.hpp>
#include <Celerity/UI/WindowControl.hpp>

#include <Configuration/ResourceObjectTypeManifest.hpp>

#include <Platformer/Control/ControllableComponent.hpp>
#include <Platformer/Layer/LayerSetupComponent.hpp>
#include <Platformer/Movement/CollisionShapeMovementContextComponent.hpp>
#include <Platformer/Movement/MovementComponent.hpp>
#include <Platformer/Spawn/SpawnComponent.hpp>
#include <Platformer/Team/TeamComponent.hpp>

Emergence::Resource::Object::TypeManifest GetResourceObjectTypeManifest () noexcept
{
    static Emergence::Resource::Object::TypeManifest manifest = [] ()
    {
        Emergence::Resource::Object::TypeManifest typeManifest;
        typeManifest.AddInjection ({Emergence::Celerity::PrototypeComponent::Reflect ().mapping,
                                    Emergence::Celerity::PrototypeComponent::Reflect ().descriptorId});
        typeManifest.AddInjection ({SpawnComponent::Reflect ().mapping, SpawnComponent::Reflect ().prototypeId});

        typeManifest.Register (CollisionShapeMovementContextComponent::Reflect ().mapping,
                               {CollisionShapeMovementContextComponent::Reflect ().objectId});
        typeManifest.Register (ControllableComponent::Reflect ().mapping, {ControllableComponent::Reflect ().objectId});
        typeManifest.Register (Emergence::Celerity::ButtonControl::Reflect ().mapping,
                               {Emergence::Celerity::ButtonControl::Reflect ().nodeId});
        typeManifest.Register (Emergence::Celerity::Camera2dComponent::Reflect ().mapping,
                               {Emergence::Celerity::Camera2dComponent::Reflect ().objectId});
        typeManifest.Register (Emergence::Celerity::CheckboxControl::Reflect ().mapping,
                               {Emergence::Celerity::CheckboxControl::Reflect ().nodeId});
        typeManifest.Register (Emergence::Celerity::CollisionShape2dComponent::Reflect ().mapping,
                               {Emergence::Celerity::CollisionShape2dComponent::Reflect ().shapeId});
        typeManifest.Register (Emergence::Celerity::ContainerControl::Reflect ().mapping,
                               {Emergence::Celerity::ContainerControl::Reflect ().nodeId});
        typeManifest.Register (Emergence::Celerity::ImageControl::Reflect ().mapping,
                               {Emergence::Celerity::ImageControl::Reflect ().nodeId});
        typeManifest.Register (Emergence::Celerity::InputControl::Reflect ().mapping,
                               {Emergence::Celerity::InputControl::Reflect ().nodeId});
        typeManifest.Register (Emergence::Celerity::InputSubscriptionComponent::Reflect ().mapping,
                               {Emergence::Celerity::InputSubscriptionComponent::Reflect ().objectId});
        typeManifest.Register (Emergence::Celerity::LabelControl::Reflect ().mapping,
                               {Emergence::Celerity::LabelControl::Reflect ().nodeId});
        typeManifest.Register (Emergence::Celerity::PrototypeComponent::Reflect ().mapping,
                               {Emergence::Celerity::PrototypeComponent::Reflect ().objectId});
        typeManifest.Register (Emergence::Celerity::RigidBody2dComponent::Reflect ().mapping,
                               {Emergence::Celerity::RigidBody2dComponent::Reflect ().objectId});
        typeManifest.Register (Emergence::Celerity::Sprite2dComponent::Reflect ().mapping,
                               {Emergence::Celerity::Sprite2dComponent::Reflect ().spriteId});
        typeManifest.Register (Emergence::Celerity::Transform2dComponent::Reflect ().mapping,
                               {Emergence::Celerity::Transform2dComponent::Reflect ().objectId});
        typeManifest.Register (Emergence::Celerity::UINode::Reflect ().mapping,
                               {Emergence::Celerity::UINode::Reflect ().nodeId});
        typeManifest.Register (Emergence::Celerity::UIStyleColorProperty::Reflect ().mapping,
                               {Emergence::Celerity::UIStyleColorProperty::Reflect ().styleId});
        typeManifest.Register (Emergence::Celerity::UIStyleFloatPairProperty::Reflect ().mapping,
                               {Emergence::Celerity::UIStyleFloatPairProperty::Reflect ().styleId});
        typeManifest.Register (Emergence::Celerity::UIStyleFloatProperty::Reflect ().mapping,
                               {Emergence::Celerity::UIStyleFloatProperty::Reflect ().styleId});
        typeManifest.Register (Emergence::Celerity::UIStyleFontProperty::Reflect ().mapping,
                               {Emergence::Celerity::UIStyleFontProperty::Reflect ().styleId});
        typeManifest.Register (Emergence::Celerity::WindowControl::Reflect ().mapping,
                               {Emergence::Celerity::WindowControl::Reflect ().nodeId});
        typeManifest.Register (LayerSetupComponent::Reflect ().mapping, {LayerSetupComponent::Reflect ().objectId});
        typeManifest.Register (MovementComponent::Reflect ().mapping, {MovementComponent::Reflect ().objectId});
        typeManifest.Register (SpawnComponent::Reflect ().mapping, {SpawnComponent::Reflect ().objectId});
        typeManifest.Register (TeamComponent::Reflect ().mapping, {TeamComponent::Reflect ().objectId});
        return typeManifest;
    }();

    return manifest;
}
