#include <Celerity/Physics/Test/Lifetime.hpp>
#include <Celerity/Physics/Test/Task.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
bool LifetimeTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Celerity::Test

using namespace Emergence::Memory::Literals;
using namespace Emergence::Celerity;
using namespace Emergence::Celerity::Test::ConfiguratorTasks;
using namespace Emergence::Celerity::Test::ValidatorTasks;

BEGIN_SUITE (PhysicsLifetime)

TEST_CASE (NormalAddition)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},
                                 AddCollisionShape {{1u, 0u, "Test"_us}},
                                 AddCollisionShape {{2u, 0u, "Test"_us}},
                                 AddRigidBody {{1u}},
                                 AddTransform {{1u}},
                             }}},
                           {{0u,
                             {
                                 CheckRigidBodyExistence {0u, true},
                                 CheckRigidBodyExistence {1u, true},
                                 CheckCollisionShapeExistence {0u, true},
                                 CheckCollisionShapeExistence {1u, true},
                                 CheckCollisionShapeExistence {2u, true},
                             }}});
}

TEST_CASE (AdditionWithoutTransform)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddRigidBody {{0u}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},
                                 AddCollisionShape {{1u, 0u, "Test"_us}},
                                 AddCollisionShape {{2u, 0u, "Test"_us}},
                                 AddRigidBody {{1u}},
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

TEST_CASE (AdditionWithoutMaterial)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},
                                 AddCollisionShape {{1u, 0u, "Test"_us}},
                                 AddCollisionShape {{2u, 0u, "Test"_us}},
                             }}},
                           {{0u,
                             {
                                 CheckRigidBodyExistence {0u, true},
                                 CheckCollisionShapeExistence {0u, false},
                                 CheckCollisionShapeExistence {1u, false},
                                 CheckCollisionShapeExistence {2u, false},
                             }}});
}

TEST_CASE (CleanupAfterTransformRemoval)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},
                                 AddCollisionShape {{1u, 0u, "Test"_us}},
                                 AddTransform {{1u}},
                                 AddRigidBody {{1u}},
                             }},
                            {1u, {RemoveTransform {0u}}}},
                           {{0u,
                             {
                                 CheckRigidBodyExistence {0u, true},
                                 CheckRigidBodyExistence {1u, true},
                                 CheckCollisionShapeExistence {0u, true},
                                 CheckCollisionShapeExistence {1u, true},
                             }},
                            {1u,
                             {
                                 CheckRigidBodyExistence {0u, false},
                                 CheckRigidBodyExistence {1u, true},
                                 CheckCollisionShapeExistence {0u, false},
                                 CheckCollisionShapeExistence {1u, false},
                             }}});
}

TEST_CASE (CleanupAfterMaterialRemoval)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test1"_us}},
                                 AddDynamicsMaterial {{"Test2"_us}},
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u}},
                                 AddCollisionShape {{0u, 0u, "Test1"_us}},
                                 AddCollisionShape {{1u, 0u, "Test1"_us}},
                                 AddCollisionShape {{2u, 0u, "Test2"_us}},
                             }},
                            {1u, {RemoveDynamicsMaterial {{"Test1"_us}}}}},
                           {{0u,
                             {
                                 CheckRigidBodyExistence {0u, true},
                                 CheckCollisionShapeExistence {0u, true},
                                 CheckCollisionShapeExistence {1u, true},
                                 CheckCollisionShapeExistence {2u, true},
                             }},
                            {1u,
                             {
                                 CheckRigidBodyExistence {0u, true},
                                 CheckCollisionShapeExistence {0u, false},
                                 CheckCollisionShapeExistence {1u, false},
                                 CheckCollisionShapeExistence {2u, true},
                             }}});
}

TEST_CASE (IncorrectlyChangeShapeMaterial)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test"_us}},
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u}},
                                 AddCollisionShape {{0u, 0u, "Test"_us}},
                             }},
                            {1u, {UpdateCollisionShape {{0u, 0u, "Unknown"_us}}}}},
                           {{0u,
                             {
                                 CheckCollisionShapeExistence {0u, true},
                             }},
                            {1u,
                             {
                                 CheckCollisionShapeExistence {0u, false},
                             }}});
}

TEST_CASE (CorrectlyChangeShapeMaterial)
{
    Test::ExecuteScenario ({{0u,
                             {
                                 AddDynamicsMaterial {{"Test1"_us}},
                                 AddDynamicsMaterial {{"Test2"_us}},
                                 AddTransform {{0u}},
                                 AddRigidBody {{0u}},
                                 AddCollisionShape {{0u, 0u, "Test1"_us}},
                             }},
                            {1u, {UpdateCollisionShape {{0u, 0u, "Test2"_us}}}}},
                           {{0u,
                             {
                                 CheckCollisionShapeExistence {0u, true},
                             }},
                            {1u,
                             {
                                 CheckCollisionShapeExistence {0u, true},
                             }}});
}

TEST_CASE (IncorrectlyChangeShapeGeometry)
{
    Test::ExecuteScenario (
        {{0u,
          {
              AddDynamicsMaterial {{"Test"_us}},
              AddTransform {{0u}},
              AddRigidBody {{0u}},
              AddCollisionShape {{0u, 0u, "Test"_us, {.type = CollisionGeometryType::SPHERE, .sphereRadius = 0.5f}}},
          }},
         {1u,
          {UpdateCollisionShape {
              {0u,
               0u,
               "Test"_us,
               {.type = CollisionGeometryType::CAPSULE, .capsuleRadius = 0.5f, .capsuleHalfHeight = 0.5f}}}}}},
        {{0u,
          {
              CheckCollisionShapeExistence {0u, true},
          }},
         {1u,
          {
              CheckCollisionShapeExistence {0u, false},
          }}});
}

TEST_CASE (CorrectlyChangeShapeGeometry)
{
    Test::ExecuteScenario (
        {{0u,
          {
              AddDynamicsMaterial {{"Test"_us}},
              AddTransform {{0u}},
              AddRigidBody {{0u}},
              AddCollisionShape {{0u, 0u, "Test"_us, {.type = CollisionGeometryType::SPHERE, .sphereRadius = 0.5f}}},
          }},
         {1u,
          {UpdateCollisionShape {
              {0u, 0u, "Test"_us, {.type = CollisionGeometryType::SPHERE, .sphereRadius = 1.25f}}}}}},
        {{0u,
          {
              CheckCollisionShapeExistence {0u, true},
          }},
         {1u,
          {
              CheckCollisionShapeExistence {0u, true},
          }}});
}

END_SUITE
