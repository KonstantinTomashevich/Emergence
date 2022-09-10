#include <Celerity/Asset/Config/Messages.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const ConfigLoadingRequest::Reflection &ConfigLoadingRequest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ConfigLoadingRequest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (forceReload);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const ConfigLoadingFinishedResponse::Reflection &ConfigLoadingFinishedResponse::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ConfigLoadingFinishedResponse);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
