#include <Celerity/Render2d/AssetUsage.hpp>
#include <Celerity/Render2d/Material2d.hpp>
#include <Celerity/Render2d/Material2dInstance.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>
#include <Celerity/Render2d/Texture2d.hpp>

namespace Emergence::Celerity
{
AssetReferenceBindingList GetRender2dAssetUsage () noexcept
{
    AssetReferenceBindingList binding {GetAssetBindingAllocationGroup ()};

    AssetReferenceBinding &materialInstanceBinding = binding.emplace_back ();
    materialInstanceBinding.objectType = Material2dInstance::Reflect ().mapping;
    materialInstanceBinding.assetUserIdField = Material2dInstance::Reflect ().assetUserId;
    materialInstanceBinding.references.emplace_back () = {Material2dInstance::Reflect ().materialId,
                                                          Material2d::Reflect ().mapping};

    AssetReferenceBinding &uniformSamplerValueBinding = binding.emplace_back ();
    uniformSamplerValueBinding.objectType = UniformSamplerValue::Reflect ().mapping;
    uniformSamplerValueBinding.assetUserIdField = UniformSamplerValue::Reflect ().assetUserId;
    uniformSamplerValueBinding.references.emplace_back () = {UniformSamplerValue::Reflect ().textureId,
                                                             Texture2d::Reflect ().mapping};

    AssetReferenceBinding &sprite2dBinding = binding.emplace_back ();
    sprite2dBinding.objectType = Sprite2dComponent::Reflect ().mapping;
    sprite2dBinding.assetUserIdField = Sprite2dComponent::Reflect ().assetUserId;
    sprite2dBinding.references.emplace_back () = {Sprite2dComponent::Reflect ().materialInstanceId,
                                                  Material2dInstance::Reflect ().mapping};

    return binding;
}
} // namespace Emergence::Celerity
