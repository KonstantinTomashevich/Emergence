#include <Celerity/Asset/Render/Foundation/MaterialInstanceLoadingState.hpp>
#include <Celerity/Render/Foundation/AssetUsage.hpp>
#include <Celerity/Render/Foundation/Material.hpp>
#include <Celerity/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>

namespace Emergence::Celerity
{
void GetRenderFoundationAssetUsage (AssetReferenceBindingList &_output) noexcept
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
}
} // namespace Emergence::Celerity
