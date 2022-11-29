#include <Celerity/Asset/Render2d/MaterialInstanceLoadingState.hpp>
#include <Celerity/Render2d/AssetUsage.hpp>
#include <Celerity/Render2d/Material.hpp>
#include <Celerity/Render2d/MaterialInstance.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>
#include <Celerity/Render2d/Texture.hpp>

namespace Emergence::Celerity
{
void GetRender2dAssetUsage (AssetReferenceBindingList &_output) noexcept
{
    AssetReferenceBinding &materialInstanceBinding = _output.emplace_back ();
    materialInstanceBinding.objectType = MaterialInstance::Reflect ().mapping;
    materialInstanceBinding.assetUserIdField = MaterialInstance::Reflect ().assetUserId;
    materialInstanceBinding.references.emplace_back () = {MaterialInstance::Reflect ().materialId,
                                                          Material::Reflect ().mapping};

    AssetReferenceBinding &materialInstanceLoadingStateBinding = _output.emplace_back ();
    materialInstanceLoadingStateBinding.objectType = MaterialInstanceLoadingState::Reflect ().mapping;
    materialInstanceLoadingStateBinding.assetUserIdField = MaterialInstanceLoadingState::Reflect ().assetUserId;
    materialInstanceLoadingStateBinding.references.emplace_back () = {MaterialInstanceLoadingState::Reflect ().parentId,
                                                                      MaterialInstance::Reflect ().mapping};

    AssetReferenceBinding &uniformSamplerValueBinding = _output.emplace_back ();
    uniformSamplerValueBinding.objectType = UniformSamplerValue::Reflect ().mapping;
    uniformSamplerValueBinding.assetUserIdField = UniformSamplerValue::Reflect ().assetUserId;
    uniformSamplerValueBinding.references.emplace_back () = {UniformSamplerValue::Reflect ().textureId,
                                                             Texture::Reflect ().mapping};

    AssetReferenceBinding &sprite2dBinding = _output.emplace_back ();
    sprite2dBinding.objectType = Sprite2dComponent::Reflect ().mapping;
    sprite2dBinding.assetUserIdField = Sprite2dComponent::Reflect ().assetUserId;
    sprite2dBinding.references.emplace_back () = {Sprite2dComponent::Reflect ().materialInstanceId,
                                                  MaterialInstance::Reflect ().mapping};
}
} // namespace Emergence::Celerity
