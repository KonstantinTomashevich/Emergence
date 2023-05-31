#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>
#include <Celerity/Asset/Render/Foundation/Material.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Asset/Render/Foundation/Texture.hpp>
#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Locale/LocaleConfiguration.hpp>

#include <Configuration/ResourceConfigTypeMeta.hpp>
#include <Configuration/ResourceObjectTypeManifest.hpp>
#include <Configuration/ResourceProviderTypes.hpp>

#include <Resource/Object/Object.hpp>

#include <Root/LevelsConfigurationSingleton.hpp>

const Emergence::Container::MappingRegistry &GetResourceTypesRegistry () noexcept
{
    static Emergence::Container::MappingRegistry resourceTypesRegistry = [] ()
    {
        Emergence::Container::MappingRegistry registry;
        registry.Register (Emergence::Celerity::FontAsset::Reflect ().mapping);
        registry.Register (Emergence::Celerity::LocaleConfiguration::Reflect ().mapping);
        registry.Register (Emergence::Celerity::MaterialAsset::Reflect ().mapping);
        registry.Register (Emergence::Celerity::MaterialInstanceAsset::Reflect ().mapping);
        registry.Register (Emergence::Celerity::Sprite2dUvAnimationAsset::Reflect ().mapping);
        registry.Register (Emergence::Celerity::TextureAsset::Reflect ().mapping);
        registry.Register (Emergence::Resource::Object::Object::Reflect ().mapping);
        registry.Register (LevelsConfigurationSingleton::Reflect ().mapping);

        for (const Emergence::Celerity::ResourceConfigTypeMeta &meta : GetResourceConfigTypeMeta ())
        {
            registry.Register (meta.mapping);
        }
        return registry;
    }();

    return resourceTypesRegistry;
}

const Emergence::Container::MappingRegistry &GetPatchableTypesRegistry () noexcept
{
    static Emergence::Container::MappingRegistry patchableTypesRegistry = [] ()
    {
        Emergence::Container::MappingRegistry registry;
        Emergence::Resource::Object::TypeManifest manifest = GetResourceObjectTypeManifest ();

        for (const auto &[mapping, data] : manifest.GetMap ())
        {
            registry.Register (mapping);
        }

        return registry;
    }();

    return patchableTypesRegistry;
}
