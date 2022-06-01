#include <Physics/PhysXWorld.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Physics
{
PhysicsWorldSingleton::PhysicsWorldSingleton () noexcept
{
    new (implementationBlock.data ()) PhysXWorld ();
}

PhysicsWorldSingleton::~PhysicsWorldSingleton () noexcept
{
    block_cast<PhysXWorld> (implementationBlock).~PhysXWorld ();
}

uintptr_t PhysicsWorldSingleton::GenerateShapeUID () const noexcept
{
    assert (shapeUIDCounter != std::numeric_limits<decltype (shapeUIDCounter)>::max ());
    return const_cast<PhysicsWorldSingleton *> (this)->shapeUIDCounter++;
}

const PhysicsWorldSingleton::Reflection &PhysicsWorldSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PhysicsWorldSingleton)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (gravity)
        EMERGENCE_MAPPING_REGISTER_REGULAR (toleranceLength)
        EMERGENCE_MAPPING_REGISTER_REGULAR (toleranceSpeed)
        EMERGENCE_MAPPING_REGISTER_REGULAR (enableRemoteDebugger)
        EMERGENCE_MAPPING_REGISTER_STRING (remoteDebuggerUrl)
        EMERGENCE_MAPPING_REGISTER_REGULAR (remoteDebuggerPort)
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (collisionMasks)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Physics
