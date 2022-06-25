#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/AlignmentComponent.hpp>
#include <Gameplay/Assembly.hpp>
#include <Gameplay/PrototypeComponent.hpp>
#include <Gameplay/Spawn.hpp>
#include <Gameplay/SpawnComponent.hpp>

#include <Log/Log.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Spawn
{
using namespace Emergence::Memory::Literals;

class SpawnProcessor final : public Emergence::Celerity::TaskExecutorBase<SpawnProcessor>
{
public:
    SpawnProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;

    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedEvents;

    Emergence::Celerity::RemoveValueQuery removeSpawnById;
    Emergence::Celerity::ModifyAscendingRangeQuery modifySpawnByCoolingDownUntil;
    std::array<Emergence::Celerity::EditValueQuery, SpawnComponent::MAX_OBJECTS_PER_SPAWN> editSpawnBySpawnedObject;

    Emergence::Celerity::FetchValueQuery fetchAlignmentById;
    Emergence::Celerity::FetchValueQuery fetchTransformById;
    Emergence::Transform::Transform3dWorldAccessor transformWorldAccessor;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertPrototype;
    Emergence::Celerity::InsertLongTermQuery insertAlignment;
};

SpawnProcessor::SpawnProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),

      fetchTransformRemovedEvents (FETCH_SEQUENCE (Emergence::Transform::Transform3dComponentRemovedFixedEvent)),

      removeSpawnById (REMOVE_VALUE_1F (SpawnComponent, objectId)),
      modifySpawnByCoolingDownUntil (MODIFY_ASCENDING_RANGE (SpawnComponent, spawnCoolingDownUntilNs)),

#define INDEXED_QUERY(Index) EDIT_VALUE_1F (SpawnComponent, spawnedObjects[Index])
      editSpawnBySpawnedObject ({INDEXED_QUERY (0), INDEXED_QUERY (1), INDEXED_QUERY (2), INDEXED_QUERY (3)}),
#undef INDEXED_QUERY

      fetchAlignmentById (FETCH_VALUE_1F (AlignmentComponent, objectId)),
      fetchTransformById (FETCH_VALUE_1F (Emergence::Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertTransform (INSERT_LONG_TERM (Emergence::Transform::Transform3dComponent)),
      insertPrototype (INSERT_LONG_TERM (PrototypeComponent)),
      insertAlignment (INSERT_LONG_TERM (AlignmentComponent))
{
    _constructor.DependOn (Checkpoint::SPAWN_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::SPAWN_FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::ASSEMBLY_STARTED);
}

void SpawnProcessor::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    for (auto eventCursor = fetchTransformRemovedEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        // If deleted transform was spawned, clear it from owner spawn.
        for (std::size_t queryIndex = 0u; queryIndex < SpawnComponent::MAX_OBJECTS_PER_SPAWN; ++queryIndex)
        {
            for (auto spawnCursor = editSpawnBySpawnedObject[queryIndex].Execute (&event->objectId);
                 auto *spawn = static_cast<SpawnComponent *> (*spawnCursor); ++spawnCursor)
            {
                // We can accidentally index garbage, therefore we need to check index.
                if (queryIndex < spawn->spawnedObjects.GetCount ())
                {
                    const auto position = spawn->spawnedObjects.Begin () + static_cast<ptrdiff_t> (queryIndex);
                    spawn->spawnedObjects.EraseExchangingWithLast (position);

                    // If spawn is not on cool down already, restart cool down to avoid respawning objects right away.
                    // Also, if it was maximum count of units before, restart the cool down to avoid synced spawn.
                    if (spawn->spawnCoolingDownUntilNs < time->fixedTimeNs ||
                        spawn->spawnedObjects.GetCount () == spawn->maxSpawnedObjects - 1u)
                    {
                        spawn->spawnCoolingDownUntilNs = time->fixedTimeNs + spawn->spawnCoolDownNs;
                    }
                }
            }
        }
    }

    for (auto spawnCursor = modifySpawnByCoolingDownUntil.Execute (nullptr, &time->fixedTimeNs);
         auto *spawn = static_cast<SpawnComponent *> (*spawnCursor);)
    {
        assert (spawn->maxSpawnedObjects <= SpawnComponent::MAX_OBJECTS_PER_SPAWN);
        if (spawn->spawnedObjects.GetCount () < spawn->maxSpawnedObjects)
        {
            Emergence::Math::Transform3d spawnTransform {Emergence::Math::NoInitializationFlag::Confirm ()};
            {
                auto transformCursor = fetchTransformById.Execute (&spawn->objectId);
                const auto *transform =
                    static_cast<const Emergence::Transform::Transform3dComponent *> (*transformCursor);

                if (!transform)
                {
                    EMERGENCE_LOG (ERROR, "Spawn: Unable to attach spawn feature to object with id ", spawn->objectId,
                                   " that has no Transform3dComponent!");
                    ~spawnCursor;
                    continue;
                }

                spawnTransform = transform->GetLogicalWorldTransform (transformWorldAccessor);
            }

            Emergence::Celerity::UniqueId spawnPlayerId = Emergence::Celerity::INVALID_UNIQUE_ID;
            {
                auto alignmentCursor = fetchAlignmentById.Execute (&spawn->objectId);
                if (const auto *alignment = static_cast<const AlignmentComponent *> (*alignmentCursor))
                {
                    spawnPlayerId = alignment->playerId;
                }
            }

            const Emergence::Celerity::UniqueId objectId = world->GenerateUID ();
            auto transformCursor = insertTransform.Execute ();
            auto prototypeCursor = insertPrototype.Execute ();

            auto *transform = static_cast<Emergence::Transform::Transform3dComponent *> (++transformCursor);
            transform->SetObjectId (objectId);
            transform->SetLogicalLocalTransform (spawnTransform, true);

            auto *prototype = static_cast<PrototypeComponent *> (++prototypeCursor);
            prototype->objectId = objectId;
            prototype->prototype = spawn->spawnPrototype;

            if (spawnPlayerId != Emergence::Celerity::INVALID_UNIQUE_ID)
            {
                auto alignmentCursor = insertAlignment.Execute ();
                auto *alignment = static_cast<AlignmentComponent *> (++alignmentCursor);
                alignment->objectId = objectId;
                alignment->playerId = spawnPlayerId;
            }

            spawn->spawnedObjects.EmplaceBack (objectId);
            spawn->spawnCoolingDownUntilNs = time->fixedTimeNs + spawn->spawnCoolDownNs;
        }

        ++spawnCursor;
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("Spawn::RemoveSpawns"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Transform::Transform3dComponentRemovedFixedEvent, SpawnComponent, objectId)
        .DependOn (Checkpoint::SPAWN_STARTED)
        .MakeDependencyOf ("Spawn::Process"_us);

    _pipelineBuilder.AddTask ("Spawn::Process"_us).SetExecutor<SpawnProcessor> ();
}
} // namespace Spawn
