#include <Celerity/Physics2d/Test/Simulation.hpp>
#include <Celerity/Physics2d/Test/Task.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
bool SimulationTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Celerity::Test

using namespace Emergence::Memory::Literals;
using namespace Emergence::Celerity;
using namespace Emergence::Math;
using namespace Emergence::Celerity::Test::ConfiguratorTasks;
using namespace Emergence::Celerity::Test::ValidatorTasks;

BEGIN_SUITE (PhysicsSimulation)

TEST_CASE (KinematicFixedVelocityMovement)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u, RigidBody2dType::KINEMATIC, {1.0f, 2.0f}}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},
                             }}},
                           {{static_cast<uint64_t> (roundf (2.0f / Test::TEST_FIXED_FRAME_S)),
                             {
                                 CheckObjectTransform {0u, {{2.0f, 4.0f}, 0.0f, Vector2f::ONE}},
                             }}});
}

TEST_CASE (DynamicFixedVelocityMovementNoGravity)
{
    Test::ExecuteScenario (
        {{0u,
          {
              AddDynamicsMaterial {{"Test"_us}},
              AddTransform {{0u}},
              AddRigidBody {{0u, RigidBody2dType::DYNAMIC, {1.0f, 2.0f}, 0.0f, 0.0f, 0.05f, false, false}},
              AddCollisionShape {{0u, 0u, "Test"_us}},
          }}},
        {{static_cast<uint64_t> (roundf (2.0f / Test::TEST_FIXED_FRAME_S)),
          {
              CheckObjectTransform {0u, {{2.0f, 4.0f}, 0.0f, Vector2f::ONE}},
          }}});
}

TEST_CASE (DynamicFixedVelocityRotationNoGravity)
{
    Test::ExecuteScenario (
        {{0u,
          {
              AddDynamicsMaterial {{"Test"_us}},
              AddTransform {{0u}},
              AddRigidBody {{0u, RigidBody2dType::DYNAMIC, Vector2f::ZERO, PI * 0.75f, 0.0f, 0.05f, false, false}},
              AddCollisionShape {{0u, 0u, "Test"_us}},
          }}},
        {{static_cast<uint64_t> (roundf (2.0f / Test::TEST_FIXED_FRAME_S)),
          {
              CheckObjectTransform {0u, {Vector2f::ZERO, 4.51815f, Vector2f::ONE}},
          }}});
}

TEST_CASE (DynamicImpulseNoGravity)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u,
                                                RigidBody2dType::DYNAMIC,
                                                Vector2f::ZERO,
                                                0.0f,
                                                0.0f,
                                                0.05f,
                                                false,
                                                false,
                                                false,
                                                {-10.0f, 10.0f}}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},
                             }}},
                           {{static_cast<uint64_t> (roundf (2.0f / Test::TEST_FIXED_FRAME_S)),
                             {
                                 CheckObjectTransform {0u, {{-20.15f, 20.15f}, 0.0f, Vector2f::ONE}},
                             }}});
}

TEST_CASE (DynamicImpulseNoGravityWithRotationLocking)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u,
                                                RigidBody2dType::DYNAMIC,
                                                Vector2f::ZERO,
                                                0.0f,
                                                0.0f,
                                                0.05f,
                                                false,
                                                false,
                                                false,
                                                {-10.0f, 10.0f},
                                                0.75f * PI,
                                                true}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},
                             }}},
                           {{static_cast<uint64_t> (roundf (2.0f / Test::TEST_FIXED_FRAME_S)),
                             {
                                 CheckObjectTransform {0u, {{-20.15f, 20.15f}, 0.0f, Vector2f::ONE}},
                             }}});
}

TEST_CASE (Gravity)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u, RigidBody2dType::DYNAMIC}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},
                             }}},
                           {{static_cast<uint64_t> (roundf (2.0f / Test::TEST_FIXED_FRAME_S)),
                             {
                                 CheckObjectTransform {0u, {{0.0f, -20.1f}, 0.0f, Vector2f::ONE}},
                             }}});
}

TEST_CASE (DynamicAndStaticContact)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u, {{0.0f, 10.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{0u, RigidBody2dType::DYNAMIC}},
                                 AddCollisionShape {{0u,
                                                     0u,
                                                     "Test"_us,
                                                     {.type = CollisionGeometry2dType::CIRCLE, .circleRadius = 1.0f},
                                                     Vector2f::ZERO,
                                                     0.0f,
                                                     false,
                                                     true,
                                                     true}},

                                 AddTransform {{1u, {{-0.9f, 0.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{1u, RigidBody2dType::STATIC}},
                                 AddCollisionShape {{1u, 1u, "Test"_us}},
                             }}},
                           {
                               {78u,
                                {
                                    CheckEvents {{{1u, 1u, 0u, 0u}}, {}, {}, {}},
                                }},
                               {79u,
                                {
                                    CheckEvents {{}, {}, {}, {}},
                                }},
                               {80u,
                                {
                                    CheckEvents {{}, {}, {}, {}},
                                }},
                               {81u,
                                {
                                    CheckEvents {{}, {{1u, 1u, 0u, 0u}}, {}, {}},
                                }},
                           });
}

TEST_CASE (DynamicAndDynamicContact)
{
    Test::ExecuteScenario (
        {{0u,
          {
              AddDynamicsMaterial {{"Test"_us}},
              AddTransform {{0u, {{0.0f, 10.0f}, 0.0f, Vector2f::ONE}}},
              AddRigidBody {{0u, RigidBody2dType::DYNAMIC}},
              AddCollisionShape {{0u,
                                  0u,
                                  "Test"_us,
                                  {.type = CollisionGeometry2dType::CIRCLE, .circleRadius = 1.0f},
                                  Vector2f::ZERO,
                                  0.0f,
                                  false,
                                  false,
                                  true}},

              AddTransform {{1u, {{-0.9f, 0.0f}, 0.0f, Vector2f::ONE}}},
              AddRigidBody {{1u, RigidBody2dType::DYNAMIC, {0.0f, 2.0f}, 0.0f, 0.0f, 0.05f, false, false}},
              AddCollisionShape {{1u, 1u, "Test"_us}},
          }}},
        {
            {68u,
             {
                 CheckEvents {{{1u, 1u, 0u, 0u}}, {}, {}, {}},
             }},
            {69u,
             {
                 CheckEvents {{}, {}, {}, {}},
             }},
            {70u,
             {
                 CheckEvents {{}, {{1u, 1u, 0u, 0u}}, {}, {}},
             }},
        });
}

TEST_CASE (Trigger)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u, {{0.0f, 10.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{0u, RigidBody2dType::DYNAMIC}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},

                                 AddTransform {{1u, {{-0.9f, 0.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{1u, RigidBody2dType::STATIC}},
                                 AddCollisionShape {{1u,
                                                     1u,
                                                     "Test"_us,
                                                     {.type = CollisionGeometry2dType::CIRCLE, .circleRadius = 1.0f},
                                                     Vector2f::ZERO,
                                                     0.0f,
                                                     true,
                                                     true,
                                                     false}},
                             }}},
                           {
                               {79u,
                                {
                                    CheckEvents {{}, {}, {{1u, 1u, 0u, 0u}}, {}},
                                }},
                               {92u,
                                {
                                    CheckEvents {{}, {}, {}, {{1u, 1u, 0u, 0u}}},
                                }},
                           });
}

TEST_CASE (MultishapeContact)
{
    Test::ExecuteScenario (
        {{0u,
          {
              AddDynamicsMaterial {{"Test"_us}},
              AddTransform {{0u, {{0.0f, 10.0f}, 0.0f, Vector2f::ONE}}},
              AddRigidBody {{0u, RigidBody2dType::DYNAMIC}},
              AddCollisionShape {{0u,
                                  0u,
                                  "Test"_us,
                                  {.type = CollisionGeometry2dType::CIRCLE, .circleRadius = 1.0f},
                                  Vector2f::ZERO,
                                  0.0f,
                                  false,
                                  true,
                                  true}},

              AddTransform {{1u, {{-0.9f, 0.0f}, 0.0f, Vector2f::ONE}}},
              AddRigidBody {{1u, RigidBody2dType::STATIC}},
              AddCollisionShape {
                  {1u, 1u, "Test"_us, {.type = CollisionGeometry2dType::CIRCLE, .circleRadius = 1.0f}, {-2.0f, 0.0f}}},
              AddCollisionShape {{2u, 1u, "Test"_us}},
          }}},
        {
            {78u,
             {
                 CheckEvents {{{1u, 2u, 0u, 0u}}, {}, {}, {}},
             }},
            {80u,
             {
                 CheckEvents {{}, {}, {}, {}},
             }},
            {81u,
             {
                 CheckEvents {{}, {{1u, 2u, 0u, 0u}}, {}, {}},
             }},
        });
}

TEST_CASE (CollisionMaskMismatch)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u, {{0.0f, 10.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{0u, RigidBody2dType::DYNAMIC}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},

                                 AddTransform {{1u, {{-0.9f, 0.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{1u, RigidBody2dType::STATIC}},
                                 AddCollisionShape {{1u,
                                                     1u,
                                                     "Test"_us,
                                                     {.type = CollisionGeometry2dType::CIRCLE, .circleRadius = 1.0f},
                                                     Vector2f::ZERO,
                                                     0.0f,
                                                     false,
                                                     true,
                                                     true,
                                                     1u}},
                             }}},
                           {
                               {79u,
                                {
                                    CheckEvents {{}, {}, {}, {}},
                                }},
                               {80u,
                                {
                                    CheckEvents {{}, {}, {}, {}},
                                }},
                               {81u,
                                {
                                    CheckEvents {{}, {}, {}, {}},
                                }},
                               {82u,
                                {
                                    CheckEvents {{}, {}, {}, {}},
                                }},
                               {92u,
                                {
                                    CheckEvents {{}, {}, {}, {}},
                                }},
                           });
}

TEST_CASE (MaterialUpdate)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u, {{0.0f, 10.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{0u, RigidBody2dType::DYNAMIC}},
                                 AddCollisionShape {{0u,
                                                     0u,
                                                     "Test"_us,
                                                     {.type = CollisionGeometry2dType::CIRCLE, .circleRadius = 1.0f},
                                                     Vector2f::ZERO,
                                                     0.0f,
                                                     false,
                                                     true,
                                                     true}},

                                 AddTransform {{1u, {{-0.5f, 0.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{1u, RigidBody2dType::STATIC}},
                                 AddCollisionShape {{1u, 1u, "Test"_us}},
                             }},
                            {25u,
                             {
                                 UpdateDynamicsMaterial {{"Test"_us, 0.0f, 0.75f, 0.1f, 0.75f}},
                             }}},
                           {
                               {78u,
                                {
                                    CheckEvents {{{1u, 1u, 0u, 0u}}, {}, {}, {}},
                                }},
                               {79u,
                                {
                                    CheckEvents {{}, {{1u, 1u, 0u, 0u}}, {}, {}},
                                }},
                           });
}

TEST_CASE (MaterialReplaced)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddDynamicsMaterial {{"TestChanged"_us, 0.0f, 0.5f, 0.5f, 1.0f}},

                                 AddTransform {{0u, {{0.0f, 10.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{0u, RigidBody2dType::DYNAMIC}},
                                 AddCollisionShape {{0u,
                                                     0u,
                                                     "Test"_us,
                                                     {.type = CollisionGeometry2dType::CIRCLE, .circleRadius = 1.0f},
                                                     Vector2f::ZERO,
                                                     0.0f,
                                                     false,
                                                     true,
                                                     true}},

                                 AddTransform {{1u, {{-0.5f, 0.0f}, 0.0f, Vector2f::ONE}}},
                                 AddRigidBody {{1u, RigidBody2dType::STATIC}},
                                 AddCollisionShape {{1u, 1u, "Test"_us}},
                             }},
                            {25u,
                             {
                                 UpdateCollisionShape {{0u,
                                                        0u,
                                                        "TestChanged"_us,
                                                        {.type = CollisionGeometry2dType::CIRCLE, .circleRadius = 1.0f},
                                                        Vector2f::ZERO,
                                                        0.0f,
                                                        false,
                                                        true,
                                                        true}},
                             }}},
                           {
                               {78u,
                                {
                                    CheckEvents {{{1u, 1u, 0u, 0u}}, {}, {}, {}},
                                }},
                               {79u,
                                {
                                    CheckEvents {{}, {{1u, 1u, 0u, 0u}}, {}, {}},
                                }},
                           });
}

TEST_CASE (OutsideManipulationEnabled)
{
    const auto segmentDuration = static_cast<uint64_t> (roundf (2.0f / Test::TEST_FIXED_FRAME_S));
    Test::ExecuteScenario (
        {
            {0u,
             {
                 AddDynamicsMaterial {{"Test"_us}},
                 AddTransform {{0u}},
                 AddRigidBody {{0u, RigidBody2dType::DYNAMIC, Vector2f::ZERO, 0.0f, 0.0f, 0.05f, false, true, true}},
                 AddCollisionShape {{0u, 0u, "Test"_us}},
             }},
            {segmentDuration + 1u,
             {
                 UpdateTransform {{0u}},
                 UpdateRigidBody {{0u, RigidBody2dType::DYNAMIC, {1.0f, 2.0f}, 0.0f, 0.0f, 0.05f, false, false, true}},
             }},
            {segmentDuration * 2u + 1u,
             {
                 // We are checking that updating transform on unobserved body does nothing.
                 UpdateRigidBody {{0u, RigidBody2dType::DYNAMIC, {1.0f, 2.0f}, 0.0f, 0.0f, 0.05f, false, false, false}},
                 UpdateTransform {{0u}},
             }},
        },
        {
            {segmentDuration,
             {
                 CheckObjectTransform {0u, {{0.0f, -20.1f}, 0.0f, Vector2f::ONE}},
             }},
            {segmentDuration * 2u,
             {
                 CheckObjectTransform {0u, {{2.0f, 4.0f}, 0.0f, Vector2f::ONE}},
             }},
            {segmentDuration * 3u,
             {
                 CheckObjectTransform {0u, {{4.0f, 8.0f}, 0.0f, Vector2f::ONE}},
             }},
        });
}

END_SUITE
