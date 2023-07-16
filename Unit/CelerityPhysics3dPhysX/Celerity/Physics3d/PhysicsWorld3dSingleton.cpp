#include <limits>

#include <Assert/Assert.hpp>

#include <Celerity/Physics3d/PhysXWorld.hpp>
#include <Celerity/Physics3d/PhysicsWorld3dSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Celerity
{
PhysicsWorld3dSingleton::PhysicsWorld3dSingleton () noexcept
{
    new (implementationBlock.data ()) PhysXWorld ();
}

PhysicsWorld3dSingleton::~PhysicsWorld3dSingleton () noexcept
{
    block_cast<PhysXWorld> (implementationBlock).~PhysXWorld ();
}

uintptr_t PhysicsWorld3dSingleton::GenerateShapeId () const noexcept
{
    EMERGENCE_ASSERT (shapeIdCounter != std::numeric_limits<decltype (shapeIdCounter)::value_type>::max ());
    return const_cast<PhysicsWorld3dSingleton *> (this)->shapeIdCounter++;
}

const PhysicsWorld3dSingleton::Reflection &PhysicsWorld3dSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PhysicsWorld3dSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (toleranceLength);
        EMERGENCE_MAPPING_REGISTER_REGULAR (toleranceSpeed);
        EMERGENCE_MAPPING_REGISTER_REGULAR (simulationMaxThreads);
        EMERGENCE_MAPPING_REGISTER_REGULAR (enableMemoryProfiling);
        EMERGENCE_MAPPING_REGISTER_REGULAR (gravity);
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (collisionMasks);
        EMERGENCE_MAPPING_REGISTER_REGULAR (enableRemoteDebugger);
        EMERGENCE_MAPPING_REGISTER_STRING (remoteDebuggerUrl);
        EMERGENCE_MAPPING_REGISTER_REGULAR (remoteDebuggerPort);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
