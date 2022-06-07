#include <Physics/PhysXAccessSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Physics
{
PhysXAccessSingleton::Reflection &PhysXAccessSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PhysXAccessSingleton)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Physics
