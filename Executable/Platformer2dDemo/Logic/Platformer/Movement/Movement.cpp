#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputActionComponent.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/Physics2d/CollisionContact2d.hpp>
#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>
#include <Celerity/Physics2d/Events.hpp>
#include <Celerity/Physics2d/PhysicsWorld2dSingleton.hpp>
#include <Celerity/Physics2d/RigidBody2dComponent.hpp>
#include <Celerity/Physics2d/Simulation.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <Log/Log.hpp>

#include <Math/Constants.hpp>
#include <Math/Scalar.hpp>

#include <Platformer/Input/InputActions.hpp>
#include <Platformer/Movement/CollisionShapeMovementContextComponent.hpp>
#include <Platformer/Movement/Movement.hpp>
#include <Platformer/Movement/MovementBlockedComponent.hpp>
#include <Platformer/Movement/MovementComponent.hpp>
#include <Platformer/Movement/MovementConfiguration.hpp>
#include <Platformer/Movement/MovementConfigurationSingleton.hpp>

namespace Movement
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"Movement::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"Movement::Finished"};

class MovementProcessor final : public Emergence::Celerity::TaskExecutorBase<MovementProcessor>
{
public:
    MovementProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    struct Context final
    {
        MovementComponent *movement = nullptr;
        Emergence::Celerity::RigidBody2dComponent *rigidBody = nullptr;
        const MovementConfiguration *configuration = nullptr;
        const MovementConfigurationSingleton *globalConfiguration = nullptr;
        const Emergence::Celerity::TimeSingleton *time = nullptr;
        const Emergence::Celerity::PhysicsWorld2dSingleton *physicsWorld = nullptr;

        Emergence::Math::Vector2f axisClampedDirectionInput = Emergence::Math::Vector2f::ZERO;
        bool blocked = false;
        bool rollInput = false;
        bool stateChanged;
    };

    void PrepareContext (Context &_context) noexcept;

    static MovementState RunStateMachine (const Context &_context) noexcept;

    void UpdateShapesWithMovementContext (const MovementComponent *_movement) noexcept;

    static void ApplyMovementState (const Context &_context) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::FetchSingletonQuery fetchMovementConfiguration;
    Emergence::Celerity::FetchSingletonQuery fetchPhysicsWorld;

    Emergence::Celerity::ModifyAscendingRangeQuery modifyMovementByIdAscending;
    Emergence::Celerity::FetchValueQuery fetchMovementConfigurationById;
    Emergence::Celerity::EditValueQuery editRigidBodyById;
    Emergence::Celerity::EditValueQuery editCollisionShapeByShapeId;

    Emergence::Celerity::FetchValueQuery fetchMovementBlockedById;
    Emergence::Celerity::FetchValueQuery fetchInputActionById;
    Emergence::Celerity::FetchValueQuery fetchCollisionContactByObjectId;
    Emergence::Celerity::RemoveValueQuery removeCollisionShapeMovementContextByObjectId;
    Emergence::Celerity::FetchValueQuery fetchCollisionShapeMovementContextByShapeId;
};

MovementProcessor::MovementProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      fetchMovementConfiguration (FETCH_SINGLETON (MovementConfigurationSingleton)),
      fetchPhysicsWorld (FETCH_SINGLETON (Emergence::Celerity::PhysicsWorld2dSingleton)),

      modifyMovementByIdAscending (MODIFY_ASCENDING_RANGE (MovementComponent, objectId)),
      fetchMovementConfigurationById (FETCH_VALUE_1F (MovementConfiguration, id)),
      editRigidBodyById (EDIT_VALUE_1F (Emergence::Celerity::RigidBody2dComponent, objectId)),
      editCollisionShapeByShapeId (EDIT_VALUE_1F (Emergence::Celerity::CollisionShape2dComponent, shapeId)),

      fetchMovementBlockedById (FETCH_VALUE_1F (MovementBlockedComponent, objectId)),
      fetchInputActionById (FETCH_VALUE_1F (Emergence::Celerity::InputActionComponent, objectId)),
      fetchCollisionContactByObjectId (FETCH_VALUE_1F (Emergence::Celerity::CollisionContact2d, objectId)),
      removeCollisionShapeMovementContextByObjectId (
          REMOVE_VALUE_1F (CollisionShapeMovementContextComponent, objectId)),
      fetchCollisionShapeMovementContextByShapeId (FETCH_VALUE_1F (CollisionShapeMovementContextComponent, shapeId))
{
    _constructor.DependOn (Emergence::Celerity::Assembly::Checkpoint::FINISHED);
    _constructor.DependOn (Emergence::Celerity::Input::Checkpoint::ACTION_COMPONENT_READ_ALLOWED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::Physics2dSimulation::Checkpoint::STARTED);
}

void MovementProcessor::Execute () noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    if (world->updateMode != Emergence::Celerity::WorldUpdateMode::SIMULATING)
    {
        return;
    }

    Context context;
    auto timeCursor = fetchTime.Execute ();
    context.time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto globalConfigurationCursor = fetchMovementConfiguration.Execute ();
    context.globalConfiguration = static_cast<const MovementConfigurationSingleton *> (*globalConfigurationCursor);

    auto physicsWorldCursor = fetchPhysicsWorld.Execute ();
    context.physicsWorld = static_cast<const Emergence::Celerity::PhysicsWorld2dSingleton *> (*physicsWorldCursor);

    for (auto movementCursor = modifyMovementByIdAscending.Execute (nullptr, nullptr);
         (context.movement = static_cast<MovementComponent *> (*movementCursor));)
    {
        auto configurationCursor = fetchMovementConfigurationById.Execute (&context.movement->configurationId);
        context.configuration = static_cast<const MovementConfiguration *> (*configurationCursor);

        if (!context.configuration)
        {
            EMERGENCE_LOG (ERROR, "Movement: There is no configuration \"", context.movement->configurationId,
                           "\" for MovementComponent on object with id ", context.movement->objectId,
                           ", therefore MovementComponent will be deleted.");
            ~movementCursor;
            continue;
        }

        auto rigidBodyCursor = editRigidBodyById.Execute (&context.movement->objectId);
        context.rigidBody = static_cast<Emergence::Celerity::RigidBody2dComponent *> (*rigidBodyCursor);

        if (!context.rigidBody)
        {
            EMERGENCE_LOG (
                ERROR,
                "Movement: There is no configuration RigidBody2dComponent for MovementComponent on object with id ",
                context.movement->objectId, ", therefore MovementComponent will be deleted.");
            ~movementCursor;
            continue;
        }

        // Movement is designed for dynamic bodies.
        EMERGENCE_ASSERT (context.rigidBody->type == Emergence::Celerity::RigidBody2dType::DYNAMIC);
        // Gravitation is applied manually.
        EMERGENCE_ASSERT (!context.rigidBody->affectedByGravity);
        // Velocity changes from simulation internals (like collisions) should be ignored.
        EMERGENCE_ASSERT (context.rigidBody->ignoreSimulationVelocityChange);

        PrepareContext (context);
        const MovementState newState = RunStateMachine (context);
        context.stateChanged = newState != context.movement->state;

        if (context.stateChanged)
        {
            context.movement->state = newState;
            context.movement->stateStartTimeNs = context.time->fixedTimeNs;
            UpdateShapesWithMovementContext (context.movement);
        }

        ApplyMovementState (context);
        ++movementCursor;
    }
}

void MovementProcessor::PrepareContext (Context &_context) noexcept
{
    auto movementBlockedCursor = fetchMovementBlockedById.Execute (&_context.movement->objectId);
    _context.blocked = *movementBlockedCursor;

    if (_context.blocked)
    {
        // There is no sense to spend time gathering other information
        // as block always means immediate transition to blocked state.
        return;
    }

    _context.axisClampedDirectionInput = Emergence::Math::Vector2f::ZERO;
    _context.rollInput = false;

    for (auto inputActionCursor = fetchInputActionById.Execute (&_context.movement->objectId);
         const auto *inputAction = static_cast<const Emergence::Celerity::InputActionComponent *> (*inputActionCursor);
         ++inputActionCursor)
    {
        if (inputAction->action.group == PlatformerInputActions::DIRECTED_MOVEMENT.group &&
            inputAction->action.id == PlatformerInputActions::DIRECTED_MOVEMENT.id)
        {
            _context.axisClampedDirectionInput.x += inputAction->action.real[0u];
            _context.axisClampedDirectionInput.y += inputAction->action.real[1u];
        }
        else if (inputAction->action.group == PlatformerInputActions::MOVEMENT_ROLL.group &&
                 inputAction->action.id == PlatformerInputActions::MOVEMENT_ROLL.id)
        {
            _context.rollInput = true;
        }
    }

    _context.axisClampedDirectionInput.x = Emergence::Math::Clamp (_context.axisClampedDirectionInput.x, -1.0f, 1.0f);
    _context.axisClampedDirectionInput.y = Emergence::Math::Clamp (_context.axisClampedDirectionInput.y, -1.0f, 1.0f);
    bool inAir = true;

    for (auto contactCursor = fetchCollisionContactByObjectId.Execute (&_context.movement->objectId);
         const auto *contact = static_cast<const Emergence::Celerity::CollisionContact2d *> (*contactCursor);
         ++contactCursor)
    {
        if (contact->normal.y > 0.0f)
        {
            if (auto contextCursor = fetchCollisionShapeMovementContextByShapeId.Execute (&contact->shapeId);
                const auto *shapeContext = static_cast<const CollisionShapeMovementContextComponent *> (*contextCursor))
            {
                if (shapeContext->useForGroundContactCheck)
                {
                    const float slopeRad = Emergence::Math::Abs (
                        Emergence::Math::SignedAngle (Emergence::Math::Vector2f::UP, contact->normal));
                    EMERGENCE_ASSERT (slopeRad < Emergence::Math::PI * 0.5f);
                    const float slopeDeg = Emergence::Math::ToDegrees (slopeRad);

                    if (slopeDeg <= _context.configuration->groundMaxSlopeDeg)
                    {
                        inAir = false;
                        break;
                    }
                }
            }
        }
    }

    if (inAir)
    {
        ++_context.movement->framesInAir;
        _context.movement->framesOnGround = 0u;
    }
    else
    {
        _context.movement->framesInAir = 0u;
        ++_context.movement->framesOnGround;
    }
}

MovementState MovementProcessor::RunStateMachine (const MovementProcessor::Context &_context) noexcept
{
    if (_context.blocked)
    {
        // Early exit on block to simplify state machine.
        return MovementState::BLOCKED;
    }

    MovementState previousState = _context.movement->state;
    MovementState state = _context.movement->state;
    bool stateChanged;

    auto transit = [&previousState, &state, &stateChanged] (MovementState _target)
    {
        // Assert that state machine contains no deadlocks.
        EMERGENCE_ASSERT (_target != previousState);

        previousState = state;
        state = _target;
        stateChanged = true;
    };

    do
    {
        stateChanged = false;
        switch (state)
        {
        case MovementState::IDLE:
            if (_context.movement->framesInAir > 1u)
            {
                transit (MovementState::FALL);
            }
            else if (_context.configuration->allowRoll && _context.rollInput)
            {
                transit (MovementState::ROLL);
            }
            else if (_context.configuration->allowJump &&
                     _context.axisClampedDirectionInput.y > _context.globalConfiguration->jumpActuationThreshold)
            {
                transit (MovementState::JUMP);
            }
            else if (_context.configuration->allowCrouch &&
                     _context.axisClampedDirectionInput.y <
                         -_context.globalConfiguration->crouchOrSlideActuationThreshold)
            {
                transit (MovementState::CROUCH);
            }
            else if (_context.axisClampedDirectionInput.x > _context.globalConfiguration->runActuationThreshold ||
                     _context.axisClampedDirectionInput.x < -_context.globalConfiguration->runActuationThreshold)
            {
                transit (MovementState::RUN);
            }

            break;

        case MovementState::RUN:
            if (_context.movement->framesInAir > 1u)
            {
                transit (MovementState::FALL);
            }
            else if (_context.configuration->allowRoll && _context.rollInput)
            {
                transit (MovementState::ROLL);
            }
            else if (_context.configuration->allowJump &&
                     _context.axisClampedDirectionInput.y > _context.globalConfiguration->jumpActuationThreshold)
            {
                transit (MovementState::JUMP);
            }
            else if (_context.configuration->allowSlide &&
                     _context.axisClampedDirectionInput.y <
                         -_context.globalConfiguration->crouchOrSlideActuationThreshold)
            {
                transit (MovementState::SLIDE);
            }
            else if (_context.axisClampedDirectionInput.x < _context.globalConfiguration->runActuationThreshold &&
                     _context.axisClampedDirectionInput.x > -_context.globalConfiguration->runActuationThreshold)
            {
                transit (MovementState::IDLE);
            }

            break;

        case MovementState::CROUCH:
            if (_context.movement->framesInAir > 1u)
            {
                transit (MovementState::FALL);
            }
            else if (_context.configuration->allowRoll && _context.rollInput)
            {
                transit (MovementState::ROLL);
            }
            else if (_context.axisClampedDirectionInput.y >
                     -_context.globalConfiguration->crouchOrSlideActuationThreshold)
            {
                transit (MovementState::IDLE);
            }

            break;

        case MovementState::JUMP:
        {
            if (_context.movement->state == MovementState::JUMP &&
                _context.rigidBody->linearVelocity.y < Emergence::Math::EPSILON)
            {
                transit (MovementState::FALL);
            }

            break;
        }

        case MovementState::FALL:
            if (_context.movement->framesOnGround > 1u)
            {
                transit (MovementState::IDLE);
            }

            break;

        case MovementState::ROLL:
            if (_context.movement->framesInAir > 1u)
            {
                transit (MovementState::FALL);
            }
            else if (_context.movement->state == MovementState::ROLL &&
                     _context.movement->stateStartTimeNs +
                             static_cast<uint64_t> (_context.configuration->rollDurationS * 1e9f) <=
                         _context.time->fixedTimeNs)
            {
                transit (MovementState::IDLE);
            }

            break;

        case MovementState::SLIDE:
            if (_context.movement->framesInAir > 10u)
            {
                transit (MovementState::FALL);
            }
            else if (_context.movement->state == MovementState::SLIDE &&
                     _context.movement->stateStartTimeNs +
                             static_cast<uint64_t> (_context.configuration->slideDurationS * 1e9f) <=
                         _context.time->fixedTimeNs)
            {
                transit (MovementState::IDLE);
            }

            break;

        case MovementState::BLOCKED:
            EMERGENCE_ASSERT (!_context.blocked);
            transit (MovementState::IDLE);
            break;
        }
    } while (stateChanged);

    return state;
}

void MovementProcessor::UpdateShapesWithMovementContext (const MovementComponent *_movement) noexcept
{
    const uint8_t currentStateFlag = uint8_t {1u} << static_cast<uint8_t> (_movement->state);
    for (auto contextCursor = removeCollisionShapeMovementContextByObjectId.Execute (&_movement->objectId);
         const auto *context =
             static_cast<const CollisionShapeMovementContextComponent *> (contextCursor.ReadConst ());)
    {
        if (auto shapeCursor = editCollisionShapeByShapeId.Execute (&context->shapeId);
            auto *shape = static_cast<Emergence::Celerity::CollisionShape2dComponent *> (*shapeCursor))
        {
            shape->enabled = static_cast<uint8_t> (context->supportedStates) & currentStateFlag;
            ++contextCursor;
        }
        else
        {
            ~contextCursor;
        }
    }
}

void MovementProcessor::ApplyMovementState (const MovementProcessor::Context &_context) noexcept
{
    Emergence::Math::Vector2f nextVelocity = {0.0f, _context.movement->lastMovementVelocity.y};

    // It's better not to apply gravity velocity unless we think that we're falling,
    // due to the fact that application results in "jerky" movement in some rare cases.
    if (_context.movement->framesInAir > 0u)
    {
        nextVelocity += _context.physicsWorld->gravity * _context.time->fixedDurationS;
    }
    // If we're not in the air, but our movement velocity is negative, we need to reset it.
    // Otherwise, going down on stair-like environment will result in insane downward velocity.
    else if (_context.movement->lastMovementVelocity.y < 0.0f)
    {
        nextVelocity.y = 0.0f;
    }

    auto applyAirControl = [&nextVelocity, &_context] ()
    {
        const float previousVelocityX = _context.movement->lastMovementVelocity.x;
        nextVelocity.x = previousVelocityX;
        const float velocityDelta = _context.configuration->airControlAcceleration * _context.time->fixedDurationS;

        if (_context.axisClampedDirectionInput.x > _context.globalConfiguration->runActuationThreshold &&
            previousVelocityX < _context.configuration->airControlMaxVelocity)
        {
            nextVelocity.x =
                std::min (previousVelocityX + velocityDelta, _context.configuration->airControlMaxVelocity);
        }
        else if (_context.axisClampedDirectionInput.x < -_context.globalConfiguration->runActuationThreshold &&
                 previousVelocityX > -_context.configuration->airControlMaxVelocity)
        {
            nextVelocity.x =
                std::max (previousVelocityX - velocityDelta, -_context.configuration->airControlMaxVelocity);
        }
    };

    switch (_context.movement->state)
    {
    case MovementState::IDLE:
    case MovementState::CROUCH:
    case MovementState::BLOCKED:
        break;

    case MovementState::RUN:
        if (_context.axisClampedDirectionInput.x > _context.globalConfiguration->runActuationThreshold)
        {
            nextVelocity.x = _context.configuration->runVelocity;
        }
        else if (_context.axisClampedDirectionInput.x < -_context.globalConfiguration->runActuationThreshold)
        {
            nextVelocity.x = -_context.configuration->runVelocity;
        }

        break;

    case MovementState::JUMP:
        applyAirControl ();
        if (_context.stateChanged)
        {
            nextVelocity.y = _context.configuration->jumpVelocity;
        }

        break;

    case MovementState::FALL:
        applyAirControl ();
        break;

    case MovementState::ROLL:
        if (_context.movement->lastMovementVelocity.x >= 0.0f)
        {
            nextVelocity.x = _context.configuration->rollVelocity;
        }
        else
        {
            nextVelocity.x = -_context.configuration->rollVelocity;
        }

        break;

    case MovementState::SLIDE:
        if (_context.movement->lastMovementVelocity.x >= 0.0f)
        {
            nextVelocity.x = _context.configuration->slideVelocity;
        }
        else
        {
            nextVelocity.x = -_context.configuration->slideVelocity;
        }

        break;
    }

    const Emergence::Math::Vector2f externalVelocity =
        _context.rigidBody->linearVelocity - _context.movement->lastMovementVelocity;
    _context.rigidBody->linearVelocity = externalVelocity + nextVelocity;
    _context.movement->lastMovementVelocity = nextVelocity;
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Emergence::Memory::Literals;
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Movement::RemoveBlocks"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::TransformNodeCleanupFixedEvent, MovementBlockedComponent, objectId)
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Movement::RemoveComponents"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::TransformNodeCleanupFixedEvent, MovementComponent, objectId)
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Movement::RemoveCollisionShapeMovementContext"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::TransformNodeCleanupFixedEvent,
                                CollisionShapeMovementContextComponent, objectId)
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Movement::Processor"_us).SetExecutor<MovementProcessor> ();
}
} // namespace Movement
