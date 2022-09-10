#include <Celerity/Asset/Config/LoadingStateSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const AssetConfigLoadingStateSingleton::Reflection &AssetConfigLoadingStateSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssetConfigLoadingStateSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (pathMappingLoaded);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
