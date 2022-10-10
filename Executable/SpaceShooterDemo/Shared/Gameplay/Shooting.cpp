#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Gameplay/Events.hpp>
#include <Gameplay/InputConstant.hpp>
#include <Gameplay/Mortality.hpp>
#include <Gameplay/ShooterComponent.hpp>
#include <Gameplay/Shooting.hpp>
#include <Gameplay/Spawn.hpp>

#include <Input/Input.hpp>
#include <Input/InputListenerComponent.hpp>

#include <Log/Log.hpp>

namespace Shooting
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"ShootingStarted"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"ShootingFinished"};

class ShootingProcessor final : public Emergence::Celerity::TaskExecutorBase<ShootingProcessor>
{
public:
    ShootingProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    bool TryFetchBulletTransform (Emergence::Celerity::UniqueId _shootingPointId,
                                  Emergence::Math::Transform3d &_output) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;

    Emergence::Celerity::ModifyAscendingRangeQuery modifyShootersByCoolingDownUntil;
    Emergence::Celerity::FetchValueQuery fetchInputListenerById;
    Emergence::Celerity::FetchValueQuery fetchTransformById;
    Emergence::Celerity::Transform3dWorldAccessor transformWorldAccessor;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertPrototype;
};

ShootingProcessor::ShootingProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),

      modifyShootersByCoolingDownUntil (MODIFY_ASCENDING_RANGE (ShooterComponent, coolingDownUntilNs)),
      fetchInputListenerById (FETCH_VALUE_1F (InputListenerComponent, objectId)),
      fetchTransformById (FETCH_VALUE_1F (Emergence::Celerity::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform3dComponent)),
      insertPrototype (INSERT_LONG_TERM (Emergence::Celerity::PrototypeComponent))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (Input::Checkpoint::LISTENERS_READ_ALLOWED);

    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::Assembly::Checkpoint::STARTED);

    // To resolve race condition.
    _constructor.MakeDependencyOf (Spawn::Checkpoint::STARTED);
}

void ShootingProcessor::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    for (auto shooterCursor = modifyShootersByCoolingDownUntil.Execute (nullptr, &time->fixedTimeNs);
         auto *shooter = static_cast<ShooterComponent *> (*shooterCursor);)
    {
        auto inputListenerCursor = fetchInputListenerById.Execute (&shooter->objectId);
        const auto *inputListener = static_cast<const InputListenerComponent *> (*inputListenerCursor);

        if (!inputListener)
        {
            EMERGENCE_LOG (ERROR, "Shooting: Unable to attach shooting feature to object (id ", shooter->objectId,
                           ") without InputListenerComponent!");

            ~shooterCursor;
            continue;
        }

        for (const InputAction &action : inputListener->actions)
        {
            if (action.id == InputConstant::FIRE_ACTION)
            {
                shooter->coolingDownUntilNs = time->fixedTimeNs + shooter->coolDownNs;
                Emergence::Math::Transform3d bulletTransform {Emergence::Math::NoInitializationFlag::Confirm ()};

                if (TryFetchBulletTransform (shooter->shootingPointObjectId, bulletTransform))
                {
                    const Emergence::Celerity::UniqueId bulletObjectId = world->GenerateId ();

                    auto transformCursor = insertTransform.Execute ();
                    auto *transform = static_cast<Emergence::Celerity::Transform3dComponent *> (++transformCursor);
                    transform->SetObjectId (bulletObjectId);
                    transform->SetLogicalLocalTransform (bulletTransform, true);

                    auto prototypeCursor = insertPrototype.Execute ();
                    auto *prototype = static_cast<Emergence::Celerity::PrototypeComponent *> (++prototypeCursor);
                    prototype->objectId = bulletObjectId;
                    prototype->descriptorId = shooter->bulletPrototype;
                }

                break;
            }
        }

        ++shooterCursor;
    }
}

bool ShootingProcessor::TryFetchBulletTransform (Emergence::Celerity::UniqueId _shootingPointId,
                                                 Emergence::Math::Transform3d &_output) noexcept
{
    auto transformCursor = fetchTransformById.Execute (&_shootingPointId);
    if (const auto *transform = static_cast<const Emergence::Celerity::Transform3dComponent *> (*transformCursor))
    {
        _output = transform->GetLogicalWorldTransform (transformWorldAccessor);
        return true;
    }

    return false;
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Shooting::RemoveAfterDeath"_us)
        .AS_CASCADE_REMOVER_1F (DeathFixedEvent, ShooterComponent, objectId)
        .DependOn (Checkpoint::STARTED)
        // We process deaths from the previous frame, because mortality is processed during the end of frame.
        .MakeDependencyOf (Mortality::Checkpoint::STARTED);

    _pipelineBuilder.AddTask ("Shooting::RemoveAfterTransformRemoval"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedEvent, ShooterComponent, objectId)
        .DependOn ("Shooting::RemoveAfterDeath"_us)
        .MakeDependencyOf ("Shooting::Processor"_us);

    _pipelineBuilder.AddTask ("Shooting::Processor"_us).SetExecutor<ShootingProcessor> ();
}
} // namespace Shooting
