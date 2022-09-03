#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/Physics/PhysicsWorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/PhysicsConstant.hpp>

#include <Loading/Model/PhysicsInitializationSingleton.hpp>
#include <Loading/Task/PhysicsInitialization.hpp>

namespace PhysicsInitialization
{
const Emergence::Memory::UniqueString Checkpoint::INITIALIZED {"PhysicsInitialized"};

class PhysicsInitializer final : public Emergence::Celerity::TaskExecutorBase<PhysicsInitializer>
{
public:
    PhysicsInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyState;
    Emergence::Celerity::ModifySingletonQuery modifyPhysicsWorld;
};

PhysicsInitializer::PhysicsInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyState (MODIFY_SINGLETON (PhysicsInitializationSingleton)),
      modifyPhysicsWorld (MODIFY_SINGLETON (Emergence::Celerity::PhysicsWorldSingleton))
{
    _constructor.MakeDependencyOf (Checkpoint::INITIALIZED);
}

void PhysicsInitializer::Execute () noexcept
{
    auto stateCursor = modifyState.Execute ();
    auto *state = static_cast<PhysicsInitializationSingleton *> (*stateCursor);

    if (state->finished)
    {
        return;
    }

    auto worldCursor = modifyPhysicsWorld.Execute ();
    auto *world = static_cast<Emergence::Celerity::PhysicsWorldSingleton *> (*worldCursor);

    world->enableRemoteDebugger = false;
    strcpy (world->remoteDebuggerUrl.data (), "localhost");
    world->remoteDebuggerPort = 5425u;

    world->collisionMasks[PhysicsConstant::GROUND_COLLISION_GROUP] =
        (1u << PhysicsConstant::GROUND_COLLISION_GROUP) | (1u << PhysicsConstant::OBSTACLE_COLLISION_GROUP) |
        (1u << PhysicsConstant::FIGHTER_COLLISION_GROUP) | (1u << PhysicsConstant::BULLET_COLLISION_GROUP);

    world->collisionMasks[PhysicsConstant::OBSTACLE_COLLISION_GROUP] =
        (1u << PhysicsConstant::FIGHTER_COLLISION_GROUP) | (1u << PhysicsConstant::BULLET_COLLISION_GROUP);

    world->collisionMasks[PhysicsConstant::FIGHTER_COLLISION_GROUP] =
        (1u << PhysicsConstant::GROUND_COLLISION_GROUP) | (1u << PhysicsConstant::OBSTACLE_COLLISION_GROUP) |
        (1u << PhysicsConstant::FIGHTER_COLLISION_GROUP);

    world->collisionMasks[PhysicsConstant::BULLET_COLLISION_GROUP] =
        (1u << PhysicsConstant::OBSTACLE_COLLISION_GROUP) | (1u << PhysicsConstant::HIT_BOX_COLLISION_GROUP);

    world->collisionMasks[PhysicsConstant::HIT_BOX_COLLISION_GROUP] = (1u << PhysicsConstant::BULLET_COLLISION_GROUP);
    state->finished = true;
}

using namespace Emergence::Memory::Literals;

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::INITIALIZED);
    _pipelineBuilder.AddTask ("PhysicsInitializer"_us).SetExecutor<PhysicsInitializer> ();
}
} // namespace PhysicsInitialization
