#include <Container/MappingRegistry.hpp>

#include <Celerity/Asset/Render/Foundation/Material.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Asset/Render/Foundation/Texture.hpp>
#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Locale/LocaleConfiguration.hpp>

namespace Emergence::Testing
{
Container::MappingRegistry GetSupportedResourceTypes () noexcept
{
    Container::MappingRegistry registry;
    registry.Register (Celerity::FontAsset::Reflect ().mapping);
    registry.Register (Celerity::LocaleConfiguration::Reflect ().mapping);
    registry.Register (Celerity::MaterialAsset::Reflect ().mapping);
    registry.Register (Celerity::MaterialInstanceAsset::Reflect ().mapping);
    registry.Register (Celerity::TextureAsset::Reflect ().mapping);
    return registry;
}
} // namespace Emergence::Testing
