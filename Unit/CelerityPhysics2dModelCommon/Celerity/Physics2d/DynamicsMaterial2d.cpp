#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const DynamicsMaterial2d::Reflection &DynamicsMaterial2d::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DynamicsMaterial2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (friction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (restitution);
        EMERGENCE_MAPPING_REGISTER_REGULAR (restitutionThreshold);
        EMERGENCE_MAPPING_REGISTER_REGULAR (density);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
