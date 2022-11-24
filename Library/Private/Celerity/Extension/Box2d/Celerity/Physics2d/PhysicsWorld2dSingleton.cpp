#include <Assert/Assert.hpp>

#include <Celerity/Physics2d/PhysicsWorld2dSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Celerity
{
PhysicsWorld2dSingleton::PhysicsWorld2dSingleton () noexcept = default;

PhysicsWorld2dSingleton::~PhysicsWorld2dSingleton () noexcept = default;

uintptr_t PhysicsWorld2dSingleton::GenerateShapeId () const noexcept
{
    EMERGENCE_ASSERT (shapeIdCounter != std::numeric_limits<decltype (shapeIdCounter)::value_type>::max ());
    return const_cast<PhysicsWorld2dSingleton *> (this)->shapeIdCounter++;
}

const PhysicsWorld2dSingleton::Reflection &PhysicsWorld2dSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PhysicsWorld2dSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (enableMemoryProfiling);
        EMERGENCE_MAPPING_REGISTER_REGULAR (gravity);
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (collisionMasks);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
