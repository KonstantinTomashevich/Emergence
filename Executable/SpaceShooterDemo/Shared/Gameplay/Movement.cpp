#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/Events.hpp>
#include <Gameplay/InputConstant.hpp>
#include <Gameplay/Movement.hpp>
#include <Gameplay/MovementComponent.hpp>

#include <Input/InputListenerComponent.hpp>

#include <Log/Log.hpp>

#include <Math/Scalar.hpp>

#include <Physics/RigidBodyComponent.hpp>
#include <Physics/Simulation.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Movement
{
using namespace Emergence::Memory::Literals;

class MovementUpdater : public Emergence::Celerity::TaskExecutorBase<MovementUpdater>
{
public:
    MovementUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::RemoveAscendingRangeQuery removeMovementByAscendingId;
    Emergence::Celerity::FetchValueQuery fetchInputListenerById;
    Emergence::Celerity::EditValueQuery editRigidBodyById;

    Emergence::Celerity::FetchValueQuery fetchTransformById;
    Emergence::Celerity::Transform3dWorldAccessor transformWorldAccessor;
};

MovementUpdater::MovementUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      removeMovementByAscendingId (REMOVE_ASCENDING_RANGE (MovementComponent, objectId)),
      fetchInputListenerById (FETCH_VALUE_1F (InputListenerComponent, objectId)),
      editRigidBodyById (EDIT_VALUE_1F (Emergence::Celerity::RigidBodyComponent, objectId)),

      fetchTransformById (FETCH_VALUE_1F (Emergence::Celerity::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor)
{
    _constructor.DependOn (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);
    _constructor.DependOn (Checkpoint::ASSEMBLY_FINISHED);
    _constructor.DependOn (Checkpoint::MOVEMENT_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::MOVEMENT_FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::Simulation::Checkpoint::SIMULATION_STARTED);
}

void MovementUpdater::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    for (auto movementCursor = removeMovementByAscendingId.Execute (nullptr, nullptr);
         const auto *movement = static_cast<const MovementComponent *> (movementCursor.ReadConst ());)
    {
        auto transformCursor = fetchTransformById.Execute (&movement->objectId);
        const auto *transform = static_cast<const Emergence::Celerity::Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "Movement: Unable to attach movement feature to object (id ", movement->objectId,
                           ") without input Transform3dComponent!");

            ~movementCursor;
            continue;
        }

        auto inputListenerCursor = fetchInputListenerById.Execute (&movement->objectId);
        const auto *inputListener = static_cast<const InputListenerComponent *> (*inputListenerCursor);

        if (!inputListener)
        {
            EMERGENCE_LOG (ERROR, "Movement: Unable to attach movement feature to object (id ", movement->objectId,
                           ") without InputListenerComponent!");

            ~movementCursor;
            continue;
        }

        auto bodyCursor = editRigidBodyById.Execute (&movement->objectId);
        auto *body = static_cast<Emergence::Celerity::RigidBodyComponent *> (*bodyCursor);

        if (!body)
        {
            EMERGENCE_LOG (ERROR, "Movement: Unable to attach movement feature to object (id ", movement->objectId,
                           ") without RigidBodyComponent!");

            ~movementCursor;
            continue;
        }

        Emergence::Math::Vector3f linearFactor = Emergence::Math::Vector3f::ZERO;
        Emergence::Math::Vector3f angularFactor = Emergence::Math::Vector3f::ZERO;

        for (const InputAction &action : inputListener->actions)
        {
            if (action.id == InputConstant::MOTION_FACTOR_ACTION)
            {
                linearFactor += {action.real[0u], action.real[1u], action.real[2u]};
            }
            else if (action.id == InputConstant::ROTATION_FACTOR_ACTION)
            {
                angularFactor += {action.real[0u], action.real[1u], action.real[2u]};
            }
        }

        linearFactor.NormalizeSafe ();
        angularFactor.NormalizeSafe ();

        const Emergence::Math::Transform3d &worldTransform =
            transform->GetLogicalWorldTransform (transformWorldAccessor);

        auto updateVelocity = [time, &worldTransform] (const Emergence::Math::Vector3f &_previousVelocity,
                                                       const Emergence::Math::Vector3f &_factor,
                                                       const Emergence::Math::Vector3f &_acceleration, float _maxSpeed,
                                                       uint8_t _mask)
        {
            const Emergence::Math::Vector3f acceleration =
                Emergence::Math::Rotate (_acceleration * _factor, worldTransform.rotation) * time->fixedDurationS;

            Emergence::Math::Vector3f externalVelocity = Emergence::Math::Vector3f::ZERO;
            Emergence::Math::Vector3f controlledVelocity = Emergence::Math::Vector3f::ZERO;

            for (std::size_t index = 0u; index < 3u; ++index)
            {
                if (_mask & (1u << index))
                {
                    controlledVelocity.components[index] =
                        _previousVelocity.components[index] + acceleration.components[index];
                }
                else
                {
                    externalVelocity.components[index] = _previousVelocity.components[index];
                }
            }

            const float speedSquared = controlledVelocity.LengthSquared ();
            const float maxSpeedSquared = _maxSpeed * _maxSpeed;

            if (speedSquared > maxSpeedSquared)
            {
                const float speedModifier =
                    Emergence::Math::SquareRoot (maxSpeedSquared) / Emergence::Math::SquareRoot (speedSquared);
                controlledVelocity *= speedModifier;
            }

            return controlledVelocity + externalVelocity;
        };

        body->linearVelocity = updateVelocity (body->linearVelocity, linearFactor, movement->linearAcceleration,
                                               movement->maxLinearSpeed, movement->linearVelocityMask);

        body->angularVelocity = updateVelocity (body->angularVelocity, angularFactor, movement->angularAcceleration,
                                                movement->maxAngularSpeed, movement->angularVelocityMask);
        ++movementCursor;
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Movement::RemoveAfterDeath"_us)
        .AS_CASCADE_REMOVER_1F (DeathFixedEvent, MovementComponent, objectId)
        .DependOn (Checkpoint::ASSEMBLY_FINISHED)
        .DependOn (Checkpoint::MOVEMENT_STARTED);

    _pipelineBuilder.AddTask ("Movement::RemoveAfterTransformRemoval"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedEvent, MovementComponent,
                                objectId)
        .DependOn ("Movement::RemoveAfterDeath"_us)
        .MakeDependencyOf ("Movement::Update"_us);

    _pipelineBuilder.AddTask ("Movement::Update"_us).SetExecutor<MovementUpdater> ();
}
} // namespace Movement
