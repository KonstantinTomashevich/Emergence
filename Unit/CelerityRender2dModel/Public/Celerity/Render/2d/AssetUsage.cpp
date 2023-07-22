#include <Celerity/Render/2d/AssetUsage.hpp>
#include <Celerity/Render/2d/DebugShape2dComponent.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimation.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimationComponent.hpp>
#include <Celerity/Render/Foundation/MaterialInstance.hpp>

namespace Emergence::Celerity
{
void GetRender2dAssetUsage (AssetReferenceBindingList &_output) noexcept
{
    AssetReferenceBinding &sprite2dBinding = _output.emplace_back ();
    sprite2dBinding.objectType = Sprite2dComponent::Reflect ().mapping;
    sprite2dBinding.references.emplace_back () = {Sprite2dComponent::Reflect ().materialInstanceId,
                                                  MaterialInstance::Reflect ().mapping};

    AssetReferenceBinding &sprite2dUvAnimationBinding = _output.emplace_back ();
    sprite2dUvAnimationBinding.objectType = Sprite2dUvAnimation::Reflect ().mapping;
    sprite2dUvAnimationBinding.references.emplace_back () = {Sprite2dUvAnimation::Reflect ().materialInstanceId,
                                                             MaterialInstance::Reflect ().mapping};

    AssetReferenceBinding &sprite2dUvAnimationComponentBinding = _output.emplace_back ();
    sprite2dUvAnimationComponentBinding.objectType = Sprite2dUvAnimationComponent::Reflect ().mapping;
    sprite2dUvAnimationComponentBinding.references.emplace_back () = {
        Sprite2dUvAnimationComponent::Reflect ().animationId, Sprite2dUvAnimation::Reflect ().mapping};

    AssetReferenceBinding &debugShape2dBinding = _output.emplace_back ();
    debugShape2dBinding.objectType = DebugShape2dComponent::Reflect ().mapping;
    debugShape2dBinding.references.emplace_back () = {DebugShape2dComponent::Reflect ().materialInstanceId,
                                                      MaterialInstance::Reflect ().mapping};
}
} // namespace Emergence::Celerity
