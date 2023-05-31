#include <Loading/Model/ResourceConfigTypeMeta.hpp>
#include <Loading/Model/ResourceObjectTypeManifest.hpp>
#include <Loading/Model/ResourceProviderTypes.hpp>

#include <Resource/Object/Object.hpp>

Emergence::Container::MappingRegistry PrepareResourceTypesRegistry () noexcept
{
    Emergence::Container::MappingRegistry registry;
    registry.Register (Emergence::Resource::Object::Object::Reflect ().mapping);

    for (const Emergence::Celerity::ResourceConfigTypeMeta &meta : PrepareResourceConfigTypeMeta ())
    {
        registry.Register (meta.mapping);
    }

    return registry;
}

Emergence::Container::MappingRegistry PreparePatchableTypesRegistry () noexcept
{
    Emergence::Container::MappingRegistry registry;
    Emergence::Resource::Object::TypeManifest manifest = PrepareResourceObjectTypeManifest ();
    for (const auto &[mapping, data] : manifest.GetMap ())
    {
        registry.Register (mapping);
    }

    return registry;
}
