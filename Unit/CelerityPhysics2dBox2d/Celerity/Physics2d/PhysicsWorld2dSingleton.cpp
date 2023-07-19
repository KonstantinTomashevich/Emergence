#include <API/Common/MuteWarnings.hpp>

#include <limits>

#include <Assert/Assert.hpp>

BEGIN_MUTING_WARNINGS
#define B2_USER_SETTINGS
#include <box2d/box2d.h>
END_MUTING_WARNINGS

#include <API/Common/BlockCast.hpp>

#include <Celerity/Physics2d/PhysicsWorld2dSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
// NOLINTNEXTLINE(modernize-use-equals-default): It's not actually trivial. Why linter thinks it is?
PhysicsWorld2dSingleton::PhysicsWorld2dSingleton () noexcept
{
    collisionMasks.fill (std::numeric_limits<std::uint32_t>::max ());
    block_cast<b2World *> (implementationBlock) = nullptr;
}

PhysicsWorld2dSingleton::~PhysicsWorld2dSingleton () noexcept
{
    if (auto *worldPointer = block_cast<b2World *> (implementationBlock))
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

uintptr_t PhysicsWorld2dSingleton::GenerateCollisionContactId () const noexcept
{
    EMERGENCE_ASSERT (collisionContactIdCounter !=
                      std::numeric_limits<decltype (collisionContactIdCounter)::value_type>::max ());
    return const_cast<PhysicsWorld2dSingleton *> (this)->collisionContactIdCounter++;
}

uintptr_t PhysicsWorld2dSingleton::GenerateTriggerContactId () const noexcept
{
    EMERGENCE_ASSERT (triggerContactIdCounter !=
                      std::numeric_limits<decltype (triggerContactIdCounter)::value_type>::max ());
    return const_cast<PhysicsWorld2dSingleton *> (this)->triggerContactIdCounter++;
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
