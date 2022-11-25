#include <SyntaxSugar/MuteWarnings.hpp>

#include <Assert/Assert.hpp>

BEGIN_MUTING_WARNINGS
#define B2_USER_SETTINGS
#include <box2d/box2d.h>
END_MUTING_WARNINGS

#include <Celerity/Physics2d/PhysicsWorld2dSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Celerity
{
PhysicsWorld2dSingleton::PhysicsWorld2dSingleton () noexcept
{
    block_cast<b2World *> (implementationBlock) = nullptr;
}

PhysicsWorld2dSingleton::~PhysicsWorld2dSingleton () noexcept
{
    if (auto &worldPointer = block_cast<b2World *> (implementationBlock))
    {
        worldPointer->~b2World ();
        b2Free (worldPointer);
    }
}

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
