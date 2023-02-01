#include <Resource/Object/TypeManifest.hpp>

#include <Log/Log.hpp>

namespace Emergence::Resource::Object
{
void TypeManifest::InitInjection (const StandardLayout::Mapping &_injectorType,
                                                     StandardLayout::FieldId _injectorIdField) noexcept
{
    injectorType = _injectorType;
    injectorIdField = _injectorIdField;
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

const StandardLayout::Mapping &TypeManifest::GetInjectorType () const noexcept
{
    return injectorType;
}

StandardLayout::FieldId TypeManifest::GetInjectorIdField () const noexcept
{
    return injectorIdField;
}
} // namespace Emergence::Resource::Object
