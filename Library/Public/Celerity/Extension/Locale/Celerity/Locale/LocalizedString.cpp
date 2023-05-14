#include <Celerity/Locale/LocalizedString.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const LocalizedString::Reflection &LocalizedString::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LocalizedString);
        EMERGENCE_MAPPING_REGISTER_REGULAR (key);
        EMERGENCE_MAPPING_REGISTER_REGULAR (value);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
