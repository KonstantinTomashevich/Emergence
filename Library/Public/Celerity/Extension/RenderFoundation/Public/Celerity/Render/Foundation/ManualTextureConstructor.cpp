#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/ManualTextureConstructor.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>

namespace Emergence::Celerity
{
ManualTextureConstructor::ManualTextureConstructor (TaskConstructor &_constructor) noexcept
    : insertTexture (INSERT_LONG_TERM (Texture))
{
}

void ManualTextureConstructor::ConstructManualTexture (ManualAssetConstructor &_assetConstructor,
                                                       Memory::UniqueString _assetId,
                                                       Render::Backend::Texture _texture) noexcept
{
    _assetConstructor.ConstructManualAsset (_assetId, Texture::Reflect ().mapping);
    auto cursor = insertTexture.Execute ();
    auto *texture = static_cast<Texture *> (++cursor);
    texture->assetId = _assetId;
    texture->texture = std::move (_texture);
}
} // namespace Emergence::Celerity
