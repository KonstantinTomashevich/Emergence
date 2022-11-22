#include <Celerity/Resource/Object/Messages.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const ResourceObjectRequest::Reflection &ResourceObjectRequest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceObjectRequest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (object);
        EMERGENCE_MAPPING_REGISTER_REGULAR (forceReload);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const ResourceObjectFolderRequest::Reflection &ResourceObjectFolderRequest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceObjectFolderRequest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (forceReload);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const ResourceObjectLoadedResponse::Reflection &ResourceObjectLoadedResponse::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceObjectLoadedResponse);
        EMERGENCE_MAPPING_REGISTER_REGULAR (object);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const ResourceObjectFolderLoadedResponse::Reflection &ResourceObjectFolderLoadedResponse::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceObjectFolderLoadedResponse);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
