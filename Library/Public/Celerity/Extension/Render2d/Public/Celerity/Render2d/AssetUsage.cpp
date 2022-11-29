#include <Celerity/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Render2d/AssetUsage.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>

namespace Emergence::Celerity
{
void GetRender2dAssetUsage (AssetReferenceBindingList &_output) noexcept
{
    AssetReferenceBinding &sprite2dBinding = _output.emplace_back ();
    sprite2dBinding.objectType = Sprite2dComponent::Reflect ().mapping;
    sprite2dBinding.assetUserIdField = Sprite2dComponent::Reflect ().assetUserId;
    sprite2dBinding.references.emplace_back () = {Sprite2dComponent::Reflect ().materialInstanceId,
                                                  MaterialInstance::Reflect ().mapping};
}
} // namespace Emergence::Celerity
