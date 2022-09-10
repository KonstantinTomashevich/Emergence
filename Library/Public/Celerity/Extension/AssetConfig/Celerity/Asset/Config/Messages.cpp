#include <Celerity/Asset/Config/Messages.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const AssetConfigRequest::Reflection &AssetConfigRequest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssetConfigRequest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (forceReload);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const AssetConfigLoadedResponse::Reflection &AssetConfigLoadedResponse::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssetConfigLoadedResponse);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
