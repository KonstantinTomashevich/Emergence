#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputActionComponent.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/Physics2d/CollisionContact2d.hpp>
#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>
#include <Celerity/Physics2d/Events.hpp>
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
    struct ExternalContext final
    {
        Emergence::Math::Vector2f axisClampedDirectionInput = Emergence::Math::Vector2f::ZERO;
        Emergence::Math::Vector2f bodyVelocity = Emergence::Math::Vector2f::ZERO;
        bool blocked = false;
        bool inAir = false;
        bool rollInput = false;
    };

    void GatherContext (ExternalContext &_context,
                        const MovementComponent *_movement,
                        const Emergence::Celerity::RigidBody2dComponent *_rigidBody,
                        const MovementConfiguration *_configuration) noexcept;

    static MovementState RunStateMachine (const ExternalContext &_context,
                                          const MovementComponent *_movement,
                                          const MovementConfiguration *_configuration,
                                          const MovementConfigurationSingleton *_globalConfiguration,
                                          const Emergence::Celerity::TimeSingleton *_time) noexcept;

    void UpdateShapesWithMovementContext (const MovementComponent *_movement) noexcept;

    static void ApplyMovementState (const ExternalContext &_context,
                                    MovementComponent *_movement,
                                    Emergence::Celerity::RigidBody2dComponent *_rigidBody,
                                    const MovementConfiguration *_configuration,
                                    const MovementConfigurationSingleton *_globalConfiguration,
                                    bool _stateChanged) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::FetchSingletonQuery fetchMovementConfiguration;

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
    auto worldCursor = fetchTime.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    if (world->updateMode != Emergence::Celerity::WorldUpdateMode::SIMULATING)
    {
        return;
    }
    
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto globalConfigurationCursor = fetchMovementConfiguration.Execute ();
    const auto *globalConfiguration = static_cast<const MovementConfigurationSingleton *> (*globalConfigurationCursor);
    ExternalContext context;

    for (auto movementCursor = modifyMovementByIdAscending.Execute (nullptr, nullptr);
         auto *movement = static_cast<MovementComponent *> (*movementCursor);)
    {
        auto configurationCursor = fetchMovementConfigurationById.Execute (&movement->configurationId);
        const auto *configuration = static_cast<const MovementConfiguration *> (*configurationCursor);

        if (!configuration)
        {
            EMERGENCE_LOG (ERROR, "Movement: There is no configuration \"", movement->configurationId,
                           "\" for MovementComponent on object with id ", movement->objectId,
                           ", therefore MovementComponent will be deleted.");
            ~movementCursor;
            continue;
        }

        auto rigidBodyCursor = editRigidBodyById.Execute (&movement->objectId);
        auto *rigidBody = static_cast<Emergence::Celerity::RigidBody2dComponent *> (*rigidBodyCursor);

        if (!rigidBody)
        {
            EMERGENCE_LOG (
                ERROR,
                "Movement: There is no configuration RigidBody2dComponent for MovementComponent on object with id ",
                movement->objectId, ", therefore MovementComponent will be deleted.");
            ~movementCursor;
            continue;
        }

        GatherContext (context, movement, rigidBody, configuration);
        const MovementState newState = RunStateMachine (context, movement, configuration, globalConfiguration, time);
        const bool stateChanged = newState != movement->state;

        if (stateChanged)
        {
            movement->state = newState;
            UpdateShapesWithMovementContext (movement);
        }

        ApplyMovementState (context, movement, rigidBody, configuration, globalConfiguration, stateChanged);
        ++movementCursor;
    }
}

void MovementProcessor::GatherContext (MovementProcessor::ExternalContext &_context,
                                       const MovementComponent *_movement,
                                       const Emergence::Celerity::RigidBody2dComponent *_rigidBody,
                                       const MovementConfiguration *_configuration) noexcept
{
    auto movementBlockedCursor = fetchMovementBlockedById.Execute (&_movement->objectId);
    _context.blocked = *movementBlockedCursor;

    if (_context.blocked)
    {
        // There is no sense to spend time gathering other information
        // as block always means immediate transition to blocked state.
        return;
    }

    _context.bodyVelocity = _rigidBody->linearVelocity;
    _context.axisClampedDirectionInput = Emergence::Math::Vector2f::ZERO;
    _context.rollInput = false;

    for (auto inputActionCursor = fetchInputActionById.Execute (&_movement->objectId);
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

    _context.inAir = true;
    for (auto contactCursor = fetchCollisionContactByObjectId.Execute (&_movement->objectId);
         const auto *contact = static_cast<const Emergence::Celerity::CollisionContact2d *> (*contactCursor);
         ++contactCursor)
    {
        if (contact->normal.y >= 0.0f)
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

                    if (slopeDeg <= _configuration->groundMaxSlopeDeg)
                    {
                        _context.inAir = false;
                        break;
                    }
                }
            }
        }
    }
}

MovementState MovementProcessor::RunStateMachine (const MovementProcessor::ExternalContext &_context,
                                                  const MovementComponent *_movement,
                                                  const MovementConfiguration *_configuration,
                                                  const MovementConfigurationSingleton *_globalConfiguration,
                                                  const Emergence::Celerity::TimeSingleton *_time) noexcept
{
    if (_context.blocked)
    {
        // Early exit on block to simplify state machine.
        return MovementState::BLOCKED;
    }

    MovementState state = _movement->state;
    bool stateChanged;

    auto transit = [&state, &stateChanged] (MovementState _target)
    {
        state = _target;
        stateChanged = true;
    };

    do
    {
        stateChanged = false;
        switch (state)
        {
        case MovementState::IDLE:
            if (_context.inAir)
            {
                transit (MovementState::FALL);
            }
            else if (_configuration->allowRoll && _context.rollInput)
            {
                transit (MovementState::ROLL);
            }
            else if (_configuration->allowJump &&
                     _context.axisClampedDirectionInput.y > _globalConfiguration->jumpActuationThreshold)
            {
                transit (MovementState::JUMP);
            }
            else if (_configuration->allowCrouch &&
                     _context.axisClampedDirectionInput.y < -_globalConfiguration->crouchOrSlideActuationThreshold)
            {
                transit (MovementState::CROUCH);
            }
            else if (_context.axisClampedDirectionInput.x > _globalConfiguration->runActuationThreshold ||
                     _context.axisClampedDirectionInput.x < -_globalConfiguration->runActuationThreshold)
            {
                transit (MovementState::RUN);
            }

            break;

        case MovementState::RUN:
            if (_context.inAir)
            {
                transit (MovementState::FALL);
            }
            else if (_configuration->allowRoll && _context.rollInput)
            {
                transit (MovementState::ROLL);
            }
            else if (_configuration->allowJump &&
                     _context.axisClampedDirectionInput.y > _globalConfiguration->jumpActuationThreshold)
            {
                transit (MovementState::JUMP);
            }
            else if (_configuration->allowSlide &&
                     _context.axisClampedDirectionInput.y < -_globalConfiguration->crouchOrSlideActuationThreshold)
            {
                transit (MovementState::SLIDE);
            }
            else if (_context.axisClampedDirectionInput.x < _globalConfiguration->runActuationThreshold &&
                     _context.axisClampedDirectionInput.x > -_globalConfiguration->runActuationThreshold)
            {
                transit (MovementState::IDLE);
            }

            break;

        case MovementState::CROUCH:
            if (_context.inAir)
            {
                transit (MovementState::FALL);
            }
            else if (_configuration->allowRoll && _context.rollInput)
            {
                transit (MovementState::ROLL);
            }
            else if (_context.axisClampedDirectionInput.y > -_globalConfiguration->crouchOrSlideActuationThreshold)
            {
                transit (MovementState::IDLE);
            }

            break;

        case MovementState::JUMP:
        {
            if (const bool justJumped = _movement->state != MovementState::JUMP; !justJumped)
            {
                if (!_context.inAir)
                {
                    transit (MovementState::IDLE);
                }
                else if (_context.bodyVelocity.y < 0.0f)
                {
                    transit (MovementState::FALL);
                }
            }

            break;
        }

        case MovementState::FALL:
            if (!_context.inAir)
            {
                transit (MovementState::IDLE);
            }

            break;

        case MovementState::ROLL:
            if (_context.inAir)
            {
                transit (MovementState::FALL);
            }
            else if (_movement->stateStartTimeNs + static_cast<uint64_t> (_configuration->rollDurationS * 1e9f) >=
                     _time->fixedTimeNs)
            {
                transit (MovementState::IDLE);
            }

            break;

        case MovementState::SLIDE:
            if (_context.inAir)
            {
                transit (MovementState::FALL);
            }
            else if (_movement->stateStartTimeNs + static_cast<uint64_t> (_configuration->slideDurationS * 1e9f) >=
                     _time->fixedTimeNs)
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

void MovementProcessor::ApplyMovementState (const MovementProcessor::ExternalContext &_context,
                                            MovementComponent *_movement,
                                            Emergence::Celerity::RigidBody2dComponent *_rigidBody,
                                            const MovementConfiguration *_configuration,
                                            const MovementConfigurationSingleton *_globalConfiguration,
                                            bool _stateChanged) noexcept
{
    Emergence::Math::Vector2f nextVelocity = Emergence::Math::Vector2f::ZERO;
    // In some cases, like jump, velocity is added as one-up force and
    // should be treated as external during further calculations next frame.
    Emergence::Math::Vector2f forceVelocity = Emergence::Math::Vector2f::ZERO;

    auto applyAirControl = [&nextVelocity, &_context, _configuration, _globalConfiguration] ()
    {
        if (_context.axisClampedDirectionInput.x > _globalConfiguration->runActuationThreshold)
        {
            nextVelocity.x = _configuration->airControlVelocity;
        }
        else if (_context.axisClampedDirectionInput.x < -_globalConfiguration->runActuationThreshold)
        {
            nextVelocity.x = -_configuration->airControlVelocity;
        }
    };

    switch (_movement->state)
    {
    case MovementState::IDLE:
    case MovementState::CROUCH:
    case MovementState::BLOCKED:
        break;

    case MovementState::RUN:
        if (_context.axisClampedDirectionInput.x > _globalConfiguration->runActuationThreshold)
        {
            nextVelocity.x = _configuration->runVelocity;
        }
        else if (_context.axisClampedDirectionInput.x < -_globalConfiguration->runActuationThreshold)
        {
            nextVelocity.x = -_configuration->runVelocity;
        }

        break;

    case MovementState::JUMP:
        if (_stateChanged)
        {
            forceVelocity.y = _configuration->jumpVelocity;
        }

        applyAirControl ();
        break;

    case MovementState::FALL:
        applyAirControl ();
        break;

    case MovementState::ROLL:
        if (_movement->lastMovementVelocity.x >= 0.0f)
        {
            nextVelocity.x = _configuration->rollVelocity;
        }
        else
        {
            nextVelocity.x = -_configuration->rollVelocity;
        }

        break;

    case MovementState::SLIDE:
        if (_movement->lastMovementVelocity.x >= 0.0f)
        {
            nextVelocity.x = _configuration->slideVelocity;
        }
        else
        {
            nextVelocity.x = -_configuration->slideVelocity;
        }

        break;
    }

    const Emergence::Math::Vector2f externalVelocity = _rigidBody->linearVelocity - _movement->lastMovementVelocity;
    _rigidBody->linearVelocity = externalVelocity + nextVelocity + forceVelocity;
    _movement->lastMovementVelocity = nextVelocity;
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
