#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/Physics3d/PhysicsWorld3dSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/PhysicsConstant.hpp>

#include <Loading/Task/PhysicsInitialization.hpp>

namespace PhysicsInitialization
{
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"PhysicsInitializationFinished"};

class PhysicsInitializer final : public Emergence::Celerity::TaskExecutorBase<PhysicsInitializer>
{
public:
    PhysicsInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    bool initialized = false;
    Emergence::Celerity::ModifySingletonQuery modifyPhysicsWorld;
};

PhysicsInitializer::PhysicsInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPhysicsWorld (MODIFY_SINGLETON (Emergence::Celerity::PhysicsWorld3dSingleton))
{
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void PhysicsInitializer::Execute () noexcept
{
    if (initialized)
    {
        return;
    }

    auto worldCursor = modifyPhysicsWorld.Execute ();
    auto *world = static_cast<Emergence::Celerity::PhysicsWorld3dSingleton *> (*worldCursor);

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
    initialized = true;
}

using namespace Emergence::Memory::Literals;

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("PhysicsInitialization");
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("PhysicsInitializer"_us).SetExecutor<PhysicsInitializer> ();
}
} // namespace PhysicsInitialization
