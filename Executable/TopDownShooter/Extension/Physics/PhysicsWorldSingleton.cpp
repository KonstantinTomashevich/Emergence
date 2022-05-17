#include <Physics/PhysicsWorldSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Physics
{
PhysicsWorldSingleton::~PhysicsWorldSingleton () noexcept
{
    if (implementationHandle)
    {
        // TODO: Release.
    }
}

const PhysicsWorldSingleton::Reflection &PhysicsWorldSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PhysicsWorldSingleton)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (gravity)
        EMERGENCE_MAPPING_REGISTER_REGULAR (toleranceLength)
        EMERGENCE_MAPPING_REGISTER_REGULAR (toleranceSpeed)
        EMERGENCE_MAPPING_REGISTER_REGULAR (enableVisualDebugger)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Physics
