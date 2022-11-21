#include <Celerity/Resource/Config/LoadingStateSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const ResourceConfigLoadingStateSingleton::Reflection &ResourceConfigLoadingStateSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ResourceConfigLoadingStateSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (pathMappingLoaded);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
