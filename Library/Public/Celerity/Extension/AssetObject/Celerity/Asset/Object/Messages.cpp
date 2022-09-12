#include <Celerity/Asset/Object/Messages.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const AssetObjectRequest::Reflection &AssetObjectRequest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssetObjectRequest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (object);
        EMERGENCE_MAPPING_REGISTER_REGULAR (forceReload);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const AssetObjectFolderRequest::Reflection &AssetObjectFolderRequest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssetObjectFolderRequest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (forceReload);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const AssetObjectLoadedResponse::Reflection &AssetObjectLoadedResponse::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssetObjectLoadedResponse);
        EMERGENCE_MAPPING_REGISTER_REGULAR (object);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const AssetObjectFolderLoadedResponse::Reflection &AssetObjectFolderLoadedResponse::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssetObjectFolderLoadedResponse);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
