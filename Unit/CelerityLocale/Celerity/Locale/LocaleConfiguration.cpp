#include <Celerity/Locale/LocaleConfiguration.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const LocaleConfiguration::Reflection &LocaleConfiguration::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LocaleConfiguration);
        EMERGENCE_MAPPING_REGISTER_REGULAR (strings);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
