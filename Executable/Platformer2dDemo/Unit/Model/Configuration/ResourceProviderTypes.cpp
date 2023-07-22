#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>
#include <Celerity/Asset/Render/Foundation/Material.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Asset/Render/Foundation/Texture.hpp>
#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
#include <Celerity/Locale/LocaleConfiguration.hpp>
#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>
#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>
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

#include <Configuration/ResourceProviderTypes.hpp>

#include <Resource/Object/Object.hpp>

#include <Platformer/Animation/CharacterAnimationConfiguration.hpp>
#include <Platformer/Animation/CharacterAnimationControllerComponent.hpp>
#include <Platformer/Camera/CameraContextComponent.hpp>
#include <Platformer/Control/ControllableComponent.hpp>
#include <Platformer/Layer/LayerSetupComponent.hpp>
#include <Platformer/Movement/CollisionShapeMovementContextComponent.hpp>
#include <Platformer/Movement/MovementComponent.hpp>
#include <Platformer/Movement/MovementConfiguration.hpp>
#include <Platformer/Spawn/SpawnComponent.hpp>
#include <Platformer/Team/TeamComponent.hpp>

#include <Root/LevelsConfigurationSingleton.hpp>

const Emergence::Container::MappingRegistry &GetResourceTypesRegistry () noexcept
{
    static Emergence::Container::MappingRegistry resourceTypesRegistry = [] ()
    {
        Emergence::Container::MappingRegistry registry;
        registry.Register (CharacterAnimationConfiguration::Reflect ().mapping);
        registry.Register (Emergence::Celerity::DynamicsMaterial2d::Reflect ().mapping);
        registry.Register (Emergence::Celerity::FontAsset::Reflect ().mapping);
        registry.Register (Emergence::Celerity::LocaleConfiguration::Reflect ().mapping);
        registry.Register (Emergence::Celerity::MaterialAsset::Reflect ().mapping);
        registry.Register (Emergence::Celerity::MaterialInstanceAsset::Reflect ().mapping);
        registry.Register (Emergence::Celerity::Sprite2dUvAnimationAsset::Reflect ().mapping);
        registry.Register (Emergence::Celerity::TextureAsset::Reflect ().mapping);
        registry.Register (Emergence::Resource::Object::Object::Reflect ().mapping);
        registry.Register (LevelsConfigurationSingleton::Reflect ().mapping);
        registry.Register (MovementConfiguration::Reflect ().mapping);
        return registry;
    }();

    return resourceTypesRegistry;
}

const Emergence::Container::MappingRegistry &GetPatchableTypesRegistry () noexcept
{
    static Emergence::Container::MappingRegistry patchableTypesRegistry = [] ()
    {
        Emergence::Container::MappingRegistry registry;
        registry.Register (CameraContextComponent::Reflect ().mapping);
        registry.Register (CharacterAnimationControllerComponent::Reflect ().mapping);
        registry.Register (CollisionShapeMovementContextComponent::Reflect ().mapping);
        registry.Register (ControllableComponent::Reflect ().mapping);
        registry.Register (Emergence::Celerity::ButtonControl::Reflect ().mapping);
        registry.Register (Emergence::Celerity::Camera2dComponent::Reflect ().mapping);
        registry.Register (Emergence::Celerity::CheckboxControl::Reflect ().mapping);
        registry.Register (Emergence::Celerity::CollisionShape2dComponent::Reflect ().mapping);
        registry.Register (Emergence::Celerity::ContainerControl::Reflect ().mapping);
        registry.Register (Emergence::Celerity::ImageControl::Reflect ().mapping);
        registry.Register (Emergence::Celerity::InputControl::Reflect ().mapping);
        registry.Register (Emergence::Celerity::InputSubscriptionComponent::Reflect ().mapping);
        registry.Register (Emergence::Celerity::LabelControl::Reflect ().mapping);
        registry.Register (Emergence::Celerity::PrototypeComponent::Reflect ().mapping);
        registry.Register (Emergence::Celerity::RigidBody2dComponent::Reflect ().mapping);
        registry.Register (Emergence::Celerity::Sprite2dComponent::Reflect ().mapping);
        registry.Register (Emergence::Celerity::Transform2dComponent::Reflect ().mapping);
        registry.Register (Emergence::Celerity::UINode::Reflect ().mapping);
        registry.Register (Emergence::Celerity::UIStyleColorProperty::Reflect ().mapping);
        registry.Register (Emergence::Celerity::UIStyleFloatPairProperty::Reflect ().mapping);
        registry.Register (Emergence::Celerity::UIStyleFloatProperty::Reflect ().mapping);
        registry.Register (Emergence::Celerity::UIStyleFontProperty::Reflect ().mapping);
        registry.Register (Emergence::Celerity::WindowControl::Reflect ().mapping);
        registry.Register (LayerSetupComponent::Reflect ().mapping);
        registry.Register (MovementComponent::Reflect ().mapping);
        registry.Register (SpawnComponent::Reflect ().mapping);
        registry.Register (TeamComponent::Reflect ().mapping);
        return registry;
    }();

    return patchableTypesRegistry;
}
