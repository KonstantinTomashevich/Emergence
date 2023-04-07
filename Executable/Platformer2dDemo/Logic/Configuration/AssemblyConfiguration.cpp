#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>
#include <Celerity/Physics2d/PhysicsWorld2dSingleton.hpp>
#include <Celerity/Physics2d/RigidBody2dComponent.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/Render2dSingleton.hpp>
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
#include <Celerity/UI/UISingleton.hpp>
#include <Celerity/UI/UIStyle.hpp>
#include <Celerity/UI/WindowControl.hpp>

#include <Configuration/AssemblyConfiguration.hpp>

#include <Platformer/Layer/LayerSetupComponent.hpp>
#include <Platformer/Movement/CollisionShapeMovementContextComponent.hpp>
#include <Platformer/Movement/MovementComponent.hpp>
#include <Platformer/Spawn/SpawnComponent.hpp>

static Emergence::Celerity::UniqueId GenerateShapeId (const void *_argument)
{
    return static_cast<const Emergence::Celerity::PhysicsWorld2dSingleton *> (_argument)->GenerateShapeId ();
}

static Emergence::Celerity::UniqueId GenerateSpriteId (const void *_argument)
{
    return static_cast<const Emergence::Celerity::Render2dSingleton *> (_argument)->GenerateSprite2dId ();
}

static Emergence::Celerity::UniqueId GenerateUINodeId (const void *_argument)
{
    return static_cast<const Emergence::Celerity::UISingleton *> (_argument)->GenerateNodeId ();
}

static constexpr Emergence::Celerity::UniqueId SHAPE_ID_KEY = 0u;

static constexpr Emergence::Celerity::UniqueId SPRITE_ID_KEY = 1u;

static constexpr Emergence::Celerity::UniqueId UI_NODE_ID_KEY = 2u;

Emergence::Celerity::CustomKeyVector GetAssemblerCustomKeys () noexcept
{
    Emergence::Celerity::CustomKeyVector customKeys {Emergence::Celerity::GetAssemblerConfigurationAllocationGroup ()};

    Emergence::Celerity::CustomKeyDescriptor &shapeIdKey = customKeys.emplace_back ();
    shapeIdKey.singletonProviderType = Emergence::Celerity::PhysicsWorld2dSingleton::Reflect ().mapping;
    shapeIdKey.providerFunction = GenerateShapeId;

    Emergence::Celerity::CustomKeyDescriptor &spriteIdKey = customKeys.emplace_back ();
    spriteIdKey.singletonProviderType = Emergence::Celerity::Render2dSingleton::Reflect ().mapping;
    spriteIdKey.providerFunction = GenerateSpriteId;

    Emergence::Celerity::CustomKeyDescriptor &uiNodeIdKey = customKeys.emplace_back ();
    uiNodeIdKey.singletonProviderType = Emergence::Celerity::UISingleton::Reflect ().mapping;
    uiNodeIdKey.providerFunction = GenerateUINodeId;

    return customKeys;
}

Emergence::Celerity::TypeBindingVector GetFixedAssemblerTypes () noexcept
{
    Emergence::Celerity::TypeBindingVector types {Emergence::Celerity::GetAssemblerConfigurationAllocationGroup ()};

    Emergence::Celerity::TypeDescriptor &collisionShape2dComponent = types.emplace_back ();
    collisionShape2dComponent.type = Emergence::Celerity::CollisionShape2dComponent::Reflect ().mapping;
    collisionShape2dComponent.keys.emplace_back () = {
        Emergence::Celerity::CollisionShape2dComponent::Reflect ().objectId,
        Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};
    collisionShape2dComponent.keys.emplace_back () = {
        Emergence::Celerity::CollisionShape2dComponent::Reflect ().shapeId, SHAPE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &collisionShapeMovementContextComponent = types.emplace_back ();
    collisionShapeMovementContextComponent.type = CollisionShapeMovementContextComponent::Reflect ().mapping;
    collisionShapeMovementContextComponent.keys.emplace_back () = {
        CollisionShapeMovementContextComponent::Reflect ().objectId, Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};
    collisionShapeMovementContextComponent.keys.emplace_back () = {
        CollisionShapeMovementContextComponent::Reflect ().shapeId, SHAPE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &movementComponent = types.emplace_back ();
    movementComponent.type = MovementComponent::Reflect ().mapping;
    movementComponent.keys.emplace_back () = {MovementComponent::Reflect ().objectId,
                                              Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};

    Emergence::Celerity::TypeDescriptor &transform2dComponent = types.emplace_back ();
    transform2dComponent.type = Emergence::Celerity::Transform2dComponent::Reflect ().mapping;
    transform2dComponent.keys.emplace_back () = {Emergence::Celerity::Transform2dComponent::Reflect ().objectId,
                                                 Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};
    transform2dComponent.keys.emplace_back () = {Emergence::Celerity::Transform2dComponent::Reflect ().parentObjectId,
                                                 Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};

    Emergence::Celerity::TypeDescriptor &rigidBody2dComponent = types.emplace_back ();
    rigidBody2dComponent.type = Emergence::Celerity::RigidBody2dComponent::Reflect ().mapping;
    rigidBody2dComponent.keys.emplace_back () = {Emergence::Celerity::RigidBody2dComponent::Reflect ().objectId,
                                                 Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};

    Emergence::Celerity::TypeDescriptor &prototypeComponent = types.emplace_back ();
    prototypeComponent.type = Emergence::Celerity::PrototypeComponent::Reflect ().mapping;
    prototypeComponent.keys.emplace_back () = {Emergence::Celerity::PrototypeComponent::Reflect ().objectId,
                                               Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};

    return types;
}

Emergence::Celerity::TypeBindingVector GetNormalAssemblerTypes () noexcept
{
    Emergence::Celerity::TypeBindingVector types {Emergence::Celerity::GetAssemblerConfigurationAllocationGroup ()};

    Emergence::Celerity::TypeDescriptor &buttonControl = types.emplace_back ();
    buttonControl.type = Emergence::Celerity::ButtonControl::Reflect ().mapping;
    buttonControl.keys.emplace_back () = {Emergence::Celerity::ButtonControl::Reflect ().nodeId, UI_NODE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &camera2dComponent = types.emplace_back ();
    camera2dComponent.type = Emergence::Celerity::Camera2dComponent::Reflect ().mapping;
    camera2dComponent.keys.emplace_back () = {Emergence::Celerity::Camera2dComponent::Reflect ().objectId,
                                              Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};

    Emergence::Celerity::TypeDescriptor &checkboxControl = types.emplace_back ();
    checkboxControl.type = Emergence::Celerity::CheckboxControl::Reflect ().mapping;
    checkboxControl.keys.emplace_back () = {Emergence::Celerity::CheckboxControl::Reflect ().nodeId, UI_NODE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &containerControl = types.emplace_back ();
    containerControl.type = Emergence::Celerity::ContainerControl::Reflect ().mapping;
    containerControl.keys.emplace_back () = {Emergence::Celerity::ContainerControl::Reflect ().nodeId, UI_NODE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &imageControl = types.emplace_back ();
    imageControl.type = Emergence::Celerity::ImageControl::Reflect ().mapping;
    imageControl.keys.emplace_back () = {Emergence::Celerity::ImageControl::Reflect ().nodeId, UI_NODE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &inputControl = types.emplace_back ();
    inputControl.type = Emergence::Celerity::InputControl::Reflect ().mapping;
    inputControl.keys.emplace_back () = {Emergence::Celerity::InputControl::Reflect ().nodeId, UI_NODE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &labelControl = types.emplace_back ();
    labelControl.type = Emergence::Celerity::LabelControl::Reflect ().mapping;
    labelControl.keys.emplace_back () = {Emergence::Celerity::LabelControl::Reflect ().nodeId, UI_NODE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &sprite2dComponent = types.emplace_back ();
    sprite2dComponent.type = Emergence::Celerity::Sprite2dComponent::Reflect ().mapping;
    sprite2dComponent.keys.emplace_back () = {Emergence::Celerity::Sprite2dComponent::Reflect ().objectId,
                                              Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};
    sprite2dComponent.keys.emplace_back () = {Emergence::Celerity::Sprite2dComponent::Reflect ().spriteId,
                                              SPRITE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &uiNode = types.emplace_back ();
    uiNode.type = Emergence::Celerity::UINode::Reflect ().mapping;
    uiNode.keys.emplace_back () = {Emergence::Celerity::UINode::Reflect ().nodeId, UI_NODE_ID_KEY};
    uiNode.keys.emplace_back () = {Emergence::Celerity::UINode::Reflect ().parentId, UI_NODE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &uiStyleColorProperty = types.emplace_back ();
    uiStyleColorProperty.type = Emergence::Celerity::UIStyleColorProperty::Reflect ().mapping;

    Emergence::Celerity::TypeDescriptor &uiStyleFloatProperty = types.emplace_back ();
    uiStyleFloatProperty.type = Emergence::Celerity::UIStyleFloatProperty::Reflect ().mapping;

    Emergence::Celerity::TypeDescriptor &uiStyleFloatPairProperty = types.emplace_back ();
    uiStyleFloatPairProperty.type = Emergence::Celerity::UIStyleFloatPairProperty::Reflect ().mapping;

    Emergence::Celerity::TypeDescriptor &uiStyleFontProperty = types.emplace_back ();
    uiStyleFontProperty.type = Emergence::Celerity::UIStyleFontProperty::Reflect ().mapping;

    Emergence::Celerity::TypeDescriptor &windowControl = types.emplace_back ();
    windowControl.type = Emergence::Celerity::WindowControl::Reflect ().mapping;
    windowControl.keys.emplace_back () = {Emergence::Celerity::WindowControl::Reflect ().nodeId, UI_NODE_ID_KEY};

    Emergence::Celerity::TypeDescriptor &layerSetupComponent = types.emplace_back ();
    layerSetupComponent.type = LayerSetupComponent::Reflect ().mapping;
    layerSetupComponent.keys.emplace_back () = {LayerSetupComponent::Reflect ().objectId,
                                                Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};

    Emergence::Celerity::TypeDescriptor &spawnComponent = types.emplace_back ();
    spawnComponent.type = SpawnComponent::Reflect ().mapping;
    spawnComponent.keys.emplace_back () = {SpawnComponent::Reflect ().objectId,
                                           Emergence::Celerity::ASSEMBLY_OBJECT_ID_KEY_INDEX};

    return types;
}
