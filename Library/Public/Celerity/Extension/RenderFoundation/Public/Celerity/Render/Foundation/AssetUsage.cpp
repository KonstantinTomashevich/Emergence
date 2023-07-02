#include <Celerity/Asset/Render/Foundation/MaterialInstanceLoadingState.hpp>
#include <Celerity/Render/Foundation/AssetUsage.hpp>
#include <Celerity/Render/Foundation/FrameBufferTextureUsage.hpp>
#include <Celerity/Render/Foundation/Material.hpp>
#include <Celerity/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>

namespace Emergence::Celerity
{
void GetRenderFoundationAssetUsage (AssetReferenceBindingList &_output) noexcept
{
    AssetReferenceBinding &frameBufferTextureUsageBinding = _output.emplace_back ();
    frameBufferTextureUsageBinding.objectType = FrameBufferTextureUsage::Reflect ().mapping;
    frameBufferTextureUsageBinding.references.emplace_back () = {FrameBufferTextureUsage::Reflect ().textureId,
                                                                 Texture::Reflect ().mapping};

    AssetReferenceBinding &materialInstanceBinding = _output.emplace_back ();
    materialInstanceBinding.objectType = MaterialInstance::Reflect ().mapping;
    materialInstanceBinding.references.emplace_back () = {MaterialInstance::Reflect ().materialId,
                                                          Material::Reflect ().mapping};

    AssetReferenceBinding &materialInstanceLoadingStateBinding = _output.emplace_back ();
    materialInstanceLoadingStateBinding.objectType = MaterialInstanceLoadingState::Reflect ().mapping;
    materialInstanceLoadingStateBinding.references.emplace_back () = {MaterialInstanceLoadingState::Reflect ().parentId,
                                                                      MaterialInstance::Reflect ().mapping};

    AssetReferenceBinding &uniformSamplerValueBinding = _output.emplace_back ();
    uniformSamplerValueBinding.objectType = UniformSamplerValue::Reflect ().mapping;
    uniformSamplerValueBinding.references.emplace_back () = {UniformSamplerValue::Reflect ().textureId,
                                                             Texture::Reflect ().mapping};
}
} // namespace Emergence::Celerity
