#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/Events.hpp>
#include <Gameplay/HardcodedPrototypes.hpp>
#include <Gameplay/InputConstant.hpp>
#include <Gameplay/PrototypeComponent.hpp>
#include <Gameplay/ShooterComponent.hpp>
#include <Gameplay/Shooting.hpp>

#include <Input/InputListenerComponent.hpp>

#include <Log/Log.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Shooting
{
using namespace Emergence::Memory::Literals;

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

    Emergence::Celerity::FetchSequenceQuery fetchDeathEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedEvents;

    Emergence::Celerity::ModifyAscendingRangeQuery modifyShootersByCoolingDownUntil;
    Emergence::Celerity::RemoveValueQuery removeShooterById;
    Emergence::Celerity::FetchValueQuery fetchInputListenerById;
    Emergence::Celerity::FetchValueQuery fetchTransformById;
    Emergence::Transform::Transform3dWorldAccessor transformWorldAccessor;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertPrototype;
};

ShootingProcessor::ShootingProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (_constructor.MFetchSingleton (Emergence::Celerity::TimeSingleton)),
      fetchWorld (_constructor.MFetchSingleton (Emergence::Celerity::WorldSingleton)),

      fetchDeathEvents (_constructor.MFetchSequence (DeathEvent)),
      fetchTransformRemovedEvents (
          _constructor.MFetchSequence (Emergence::Transform::Transform3dComponentRemovedFixedEvent)),

      modifyShootersByCoolingDownUntil (_constructor.MModifyAscendingRange (ShooterComponent, coolingDownUntilNs)),
      removeShooterById (_constructor.MRemoveValue1F (ShooterComponent, objectId)),
      fetchInputListenerById (_constructor.MFetchValue1F (InputListenerComponent, objectId)),
      fetchTransformById (_constructor.MFetchValue1F (Emergence::Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertTransform (_constructor.MInsertLongTerm (Emergence::Transform::Transform3dComponent)),
      insertPrototype (_constructor.MInsertLongTerm (PrototypeComponent))
{
    _constructor.DependOn (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);
    _constructor.DependOn (Checkpoint::SHOOTING_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::SHOOTING_FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::ASSEMBLY_STARTED);
}

void ShootingProcessor::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto cleanup = [this] (Emergence::Celerity::UniqueId _objectId)
    {
        auto shooterCursor = removeShooterById.Execute (&_objectId);
        if (shooterCursor.ReadConst ())
        {
            ~shooterCursor;
        }
    };

    for (auto eventCursor = fetchDeathEvents.Execute ();
         const auto *event = static_cast<const DeathEvent *> (*eventCursor); ++eventCursor)
    {
        cleanup (event->objectId);
    }

    for (auto eventCursor = fetchTransformRemovedEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        cleanup (event->objectId);
    }

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
                    const Emergence::Celerity::UniqueId bulletObjectId = world->GenerateUID ();

                    auto transformCursor = insertTransform.Execute ();
                    auto *transform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
                    transform->SetObjectId (bulletObjectId);
                    transform->SetLogicalLocalTransform (bulletTransform);

                    auto prototypeCursor = insertPrototype.Execute ();
                    auto *prototype = static_cast<PrototypeComponent *> (++prototypeCursor);
                    prototype->objectId = bulletObjectId;
                    prototype->prototype = HardcodedPrototypes::BULLET;
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
    if (const auto *transform = static_cast<const Emergence::Transform::Transform3dComponent *> (*transformCursor))
    {
        _output = transform->GetLogicalWorldTransform (transformWorldAccessor);
        return true;
    }

    return false;
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Shooting::Processor"_us).SetExecutor<ShootingProcessor> ();
}
} // namespace Shooting