#include <Celerity/Render/2d/Sprite2dUvAnimation.hpp>

#include <Configuration/AssetUsage.hpp>

#include <Platformer/Animation/CharacterAnimationConfiguration.hpp>

void GetGameAssetUsage (Emergence::Celerity::AssetReferenceBindingList &_output) noexcept
{
    Emergence::Celerity::AssetReferenceBinding &characterAnimationConfigurationBinding = _output.emplace_back ();
    characterAnimationConfigurationBinding.objectType = CharacterAnimationConfiguration::Reflect ().mapping;
    characterAnimationConfigurationBinding.references.emplace_back () = {
        CharacterAnimationConfiguration::Reflect ().crouchAnimationId,
        Emergence::Celerity::Sprite2dUvAnimation::Reflect ().mapping};
    characterAnimationConfigurationBinding.references.emplace_back () = {
        CharacterAnimationConfiguration::Reflect ().fallAnimationId,
        Emergence::Celerity::Sprite2dUvAnimation::Reflect ().mapping};
    characterAnimationConfigurationBinding.references.emplace_back () = {
        CharacterAnimationConfiguration::Reflect ().idleAnimationId,
        Emergence::Celerity::Sprite2dUvAnimation::Reflect ().mapping};
    characterAnimationConfigurationBinding.references.emplace_back () = {
        CharacterAnimationConfiguration::Reflect ().jumpAnimationId,
        Emergence::Celerity::Sprite2dUvAnimation::Reflect ().mapping};
    characterAnimationConfigurationBinding.references.emplace_back () = {
        CharacterAnimationConfiguration::Reflect ().rollAnimationId,
        Emergence::Celerity::Sprite2dUvAnimation::Reflect ().mapping};
    characterAnimationConfigurationBinding.references.emplace_back () = {
        CharacterAnimationConfiguration::Reflect ().runAnimationId,
        Emergence::Celerity::Sprite2dUvAnimation::Reflect ().mapping};
    characterAnimationConfigurationBinding.references.emplace_back () = {
        CharacterAnimationConfiguration::Reflect ().slideAnimationId,
        Emergence::Celerity::Sprite2dUvAnimation::Reflect ().mapping};
}
