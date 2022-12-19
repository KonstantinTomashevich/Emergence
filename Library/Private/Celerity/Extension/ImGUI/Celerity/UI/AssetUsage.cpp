#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>
#include <Celerity/UI/AssetUsage.hpp>
#include <Celerity/UI/ImageControl.hpp>
#include <Celerity/UI/UIStyle.hpp>

namespace Emergence::Celerity
{
void GetUIAssetUsage (AssetReferenceBindingList &_output) noexcept
{
    AssetReferenceBinding &imageControlBinding = _output.emplace_back ();
    imageControlBinding.objectType = ImageControl::Reflect ().mapping;
    imageControlBinding.assetUserIdField = ImageControl::Reflect ().assetUserId;
    imageControlBinding.references.emplace_back () = {ImageControl::Reflect ().textureId, Texture::Reflect ().mapping};

    AssetReferenceBinding &styleFontPropertyBinding = _output.emplace_back ();
    styleFontPropertyBinding.objectType = UIStyleFontProperty::Reflect ().mapping;
    styleFontPropertyBinding.assetUserIdField = UIStyleFontProperty::Reflect ().assetUserId;
    styleFontPropertyBinding.references.emplace_back () = {UIStyleFontProperty::Reflect ().fontId,
                                                           Font::Reflect ().mapping};
}
} // namespace Emergence::Celerity
