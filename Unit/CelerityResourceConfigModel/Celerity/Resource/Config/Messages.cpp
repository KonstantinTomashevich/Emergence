#include <Celerity/Resource/Config/Messages.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const ResourceConfigRequest::Reflection &ResourceConfigRequest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceConfigRequest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (forceReload);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const ResourceConfigLoadedResponse::Reflection &ResourceConfigLoadedResponse::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceConfigLoadedResponse);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
