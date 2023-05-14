#include <Celerity/Locale/LocaleSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const LocaleSingleton::Reflection &LocaleSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LocaleSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (targetLocale);
        EMERGENCE_MAPPING_REGISTER_REGULAR (loadedLocale);
        EMERGENCE_MAPPING_REGISTER_REGULAR (loadingLocale);
        EMERGENCE_MAPPING_REGISTER_REGULAR (configurationInLoading);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
