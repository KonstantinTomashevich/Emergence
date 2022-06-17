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
    Emergence::Celerity::RemoveValueQuery removeMovementById;
    Emergence::Celerity::FetchValueQuery fetchInputListenerById;
    Emergence::Celerity::EditValueQuery editRigidBodyById;

    Emergence::Celerity::FetchValueQuery fetchTransformById;
    Emergence::Transform::Transform3dWorldAccessor transformWorldAccessor;

    Emergence::Celerity::FetchSequenceQuery fetchDeathEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedEvents;
};

MovementUpdater::MovementUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (_constructor.MFetchSingleton (Emergence::Celerity::TimeSingleton)),
      removeMovementByAscendingId (_constructor.MRemoveAscendingRange (MovementComponent, objectId)),
      removeMovementById (_constructor.MRemoveValue1F (MovementComponent, objectId)),
      fetchInputListenerById (_constructor.MFetchValue1F (InputListenerComponent, objectId)),
      editRigidBodyById (_constructor.MEditValue1F (Emergence::Physics::RigidBodyComponent, objectId)),

      fetchTransformById (_constructor.MFetchValue1F (Emergence::Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchDeathEvents (_constructor.MFetchSequence (DeathEvent)),
      fetchTransformRemovedEvents (
          _constructor.MFetchSequence (Emergence::Transform::Transform3dComponentRemovedFixedEvent))
{
    _constructor.DependOn (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);
    _constructor.DependOn (Checkpoint::ASSEMBLY_FINISHED);
    _constructor.DependOn (Checkpoint::MOVEMENT_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::MOVEMENT_FINISHED);
    _constructor.MakeDependencyOf (Emergence::Physics::Simulation::Checkpoint::SIMULATION_STARTED);
}

void MovementUpdater::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto removeMovement = [this] (Emergence::Celerity::UniqueId _objectId)
    {
        auto movementCursor = removeMovementById.Execute (&_objectId);
        if (movementCursor.ReadConst ())
        {
            ~movementCursor;
        }
    };

    for (auto eventCursor = fetchDeathEvents.Execute ();
         const auto *event = static_cast<const DeathEvent *> (*eventCursor); ++eventCursor)
    {
        removeMovement (event->objectId);
    }

    for (auto eventCursor = fetchTransformRemovedEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        removeMovement (event->objectId);
    }

    for (auto movementCursor = removeMovementByAscendingId.Execute (nullptr, nullptr);
         const auto *movement = static_cast<const MovementComponent *> (movementCursor.ReadConst ());)
    {
        auto transformCursor = fetchTransformById.Execute (&movement->objectId);
        const auto *transform = static_cast<const Emergence::Transform::Transform3dComponent *> (*transformCursor);

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
        auto *body = static_cast<Emergence::Physics::RigidBodyComponent *> (*bodyCursor);

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
    _pipelineBuilder.AddTask ("Movement::Update"_us).SetExecutor<MovementUpdater> ();
}
} // namespace Movement