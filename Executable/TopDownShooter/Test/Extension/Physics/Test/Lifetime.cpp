#include <Physics/Test/Lifetime.hpp>
#include <Physics/Test/Task.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Physics::Test
{
bool LifetimeTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Physics::Test

using namespace Emergence::Memory::Literals;
using namespace Emergence::Physics::Test::ConfiguratorTasks;
using namespace Emergence::Physics::Test::ValidatorTasks;

BEGIN_SUITE (PhysicsLifetime)

TEST_CASE (UnableToAddBodiesAndShapesWithoutTransform)
{
    Emergence::Physics::Test::ExecuteScenario ({{0u,
                                                 {
                                                     AddDynamicsMaterial {"Test"_us},
                                                     AddRigidBody {0u},
                                                     AddCollisionShape {0u, 0u, "Test"_us},
                                                     AddCollisionShape {1u, 0u, "Test"_us},
                                                     AddCollisionShape {2u, 0u, "Test"_us},
                                                     AddRigidBody {1u},
                                                 }}},
                                               {{0u,
                                                 {
                                                     CheckRigidBodyExistence {0u, false},
                                                     CheckRigidBodyExistence {1u, false},
                                                     CheckCollisionShapeExistence {0u, false},
                                                     CheckCollisionShapeExistence {1u, false},
                                                     CheckCollisionShapeExistence {2u, false},
                                                 }}});
}

END_SUITE
