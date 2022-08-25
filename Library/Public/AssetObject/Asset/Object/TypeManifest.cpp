#include <Asset/Object/TypeManifest.hpp>

#include <Log/Log.hpp>

namespace Emergence::Asset::Object
{
void TypeManifest::Register (const StandardLayout::Mapping &_mapping, const TypeInfo &_info) noexcept
{
    if (!infos.emplace (_mapping, _info).second)
    {
        EMERGENCE_LOG (ERROR, "TypeManifest: Detected attempt to register type \"", _mapping.GetName (),
                       "\" more than once!");
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
} // namespace Emergence::Asset::Object