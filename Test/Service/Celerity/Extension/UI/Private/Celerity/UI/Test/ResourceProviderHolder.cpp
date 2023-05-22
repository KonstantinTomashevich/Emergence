#include <Celerity/Asset/Render/Foundation/Material.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Asset/Render/Foundation/Texture.hpp>
#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Locale/LocaleConfiguration.hpp>
#include <Celerity/UI/Test/ImplementationStrings.hpp>
#include <Celerity/UI/Test/ResourceProviderHolder.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
static Container::MappingRegistry GetAssetTypes () noexcept
{
    Container::MappingRegistry registry;
    registry.Register (FontAsset::Reflect ().mapping);
    registry.Register (LocaleConfiguration::Reflect ().mapping);
    registry.Register (MaterialAsset::Reflect ().mapping);
    registry.Register (MaterialInstanceAsset::Reflect ().mapping);
    registry.Register (TextureAsset::Reflect ().mapping);
    return registry;
}

ResourceProviderHolder::ResourceProviderHolder () noexcept
    : provider (GetAssetTypes (), {})
{
    REQUIRE (provider.AddSource (Emergence::Memory::UniqueString {"Resources"}) ==
             Resource::Provider::SourceOperationResponse::SUCCESSFUL);
}

Resource::Provider::ResourceProvider &GetSharedResourceProvider () noexcept
{
    static ResourceProviderHolder holder;
    return holder.provider;
}
} // namespace Emergence::Celerity::Test
