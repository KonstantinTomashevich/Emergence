#include <Resource/Object/TypeManifest.hpp>

#include <Log/Log.hpp>

namespace Emergence::Resource::Object
{
void TypeManifest::AddInjection (DependencyInjectionInfo _info) noexcept
{
    injections.emplace_back (std::move (_info));
}

void TypeManifest::Register (const StandardLayout::Mapping &_mapping, const TypeInfo &_info) noexcept
{
    if (!infos.emplace (_mapping, _info).second)
    {
        EMERGENCE_LOG (ERROR, "Resource::Object::TypeManifest: Detected attempt to register type \"",
                       _mapping.GetName (), "\" more than once!");
    }
}

const TypeInfo *TypeManifest::Get (const StandardLayout::Mapping &_mapping) const noexcept
{
    if (auto iterator = infos.find (_mapping); iterator != infos.end ())
    {
        return &iterator->second;
    }

    return nullptr;
}

const Container::HashMap<StandardLayout::Mapping, TypeInfo> &TypeManifest::GetMap () const noexcept
{
    return infos;
}

const Container::Vector<DependencyInjectionInfo> &TypeManifest::GetInjections () const noexcept
{
    return injections;
}
} // namespace Emergence::Resource::Object
