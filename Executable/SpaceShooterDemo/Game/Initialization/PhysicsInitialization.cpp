#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/PhysicsConstant.hpp>

#include <Initialization/PhysicsInitialization.hpp>

#include <Physics/DynamicsMaterial.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>

namespace PhysicsInitialization
{
const Emergence::Memory::UniqueString Checkpoint::PHYSICS_INITIALIZED {"PhysicsInitialized"};

class PhysicsInitializer final : public Emergence::Celerity::TaskExecutorBase<PhysicsInitializer>
{
public:
    PhysicsInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyPhysicsWorld;
    Emergence::Celerity::InsertLongTermQuery insertDynamicsMaterial;
};

PhysicsInitializer::PhysicsInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyPhysicsWorld (MODIFY_SINGLETON (Emergence::Physics::PhysicsWorldSingleton)),
      insertDynamicsMaterial (INSERT_LONG_TERM (Emergence::Physics::DynamicsMaterial))
{
    _constructor.MakeDependencyOf (Checkpoint::PHYSICS_INITIALIZED);
}

void PhysicsInitializer::Execute () noexcept
{
    auto worldCursor = modifyPhysicsWorld.Execute ();
    auto *world = static_cast<Emergence::Physics::PhysicsWorldSingleton *> (*worldCursor);

    world->enableRemoteDebugger = false;
    strcpy (world->remoteDebuggerUrl.data (), "localhost");
    world->remoteDebuggerPort = 5425u;

    world->collisionMasks[PhysicsConstant::GROUND_COLLISION_GROUP] =
        (1u << PhysicsConstant::GROUND_COLLISION_GROUP) | (1u << PhysicsConstant::OBSTACLE_COLLISION_GROUP) |
        (1u << PhysicsConstant::WARRIOR_COLLISION_GROUP) | (1u << PhysicsConstant::BULLET_COLLISION_GROUP);

    world->collisionMasks[PhysicsConstant::OBSTACLE_COLLISION_GROUP] =
        (1u << PhysicsConstant::WARRIOR_COLLISION_GROUP) | (1u << PhysicsConstant::BULLET_COLLISION_GROUP);

    world->collisionMasks[PhysicsConstant::WARRIOR_COLLISION_GROUP] =
        (1u << PhysicsConstant::GROUND_COLLISION_GROUP) | (1u << PhysicsConstant::OBSTACLE_COLLISION_GROUP) |
        (1u << PhysicsConstant::WARRIOR_COLLISION_GROUP);

    world->collisionMasks[PhysicsConstant::BULLET_COLLISION_GROUP] =
        (1u << PhysicsConstant::OBSTACLE_COLLISION_GROUP) | (1u << PhysicsConstant::HIT_BOX_COLLISION_GROUP);

    world->collisionMasks[PhysicsConstant::HIT_BOX_COLLISION_GROUP] = (1u << PhysicsConstant::BULLET_COLLISION_GROUP);

    auto materialCursor = insertDynamicsMaterial.Execute ();
    auto *material = static_cast<Emergence::Physics::DynamicsMaterial *> (++materialCursor);

    material->id = PhysicsConstant::DEFAULT_MATERIAL_ID;
    material->staticFriction = 0.4f;
    material->dynamicFriction = 0.4f;
    material->enableFriction = true;
    material->restitution = 0.3f;
    material->density = 400.0f;
}

using namespace Emergence::Memory::Literals;

void AddToInitializationPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::PHYSICS_INITIALIZED);
    _pipelineBuilder.AddTask ("PhysicsInitializer"_us).SetExecutor<PhysicsInitializer> ();
}
} // namespace PhysicsInitialization
