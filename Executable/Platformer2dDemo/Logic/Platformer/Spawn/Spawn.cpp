#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Platformer/Events.hpp>
#include <Platformer/Layer/LayerSetupComponent.hpp>
#include <Platformer/Spawn/Spawn.hpp>
#include <Platformer/Spawn/SpawnComponent.hpp>
#include <Platformer/Spawn/SpawnMarkerComponent.hpp>
#include <Platformer/Team/TeamComponent.hpp>

namespace Spawn
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"Spawn::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"Spawn::Finished"};

class SpawnManager final : public Emergence::Celerity::TaskExecutorBase<SpawnManager>
{
public:
    SpawnManager (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void ProcessSpawnedObjectsRemoval (const Emergence::Celerity::TimeSingleton *_time) noexcept;

    void ProcessScheduledSpawns (const Emergence::Celerity::TimeSingleton *_time) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;

    Emergence::Celerity::FetchSequenceQuery fetchMarkerRemovedEvents;

    Emergence::Celerity::EditValueQuery editSpawnByObjectId;
    Emergence::Celerity::EditAscendingRangeQuery editSpawnByAscendingNextSpawnTimeNs;
    Emergence::Celerity::FetchValueQuery fetchLayerSetupByObjectId;
    Emergence::Celerity::FetchValueQuery fetchTeamByObjectId;

    Emergence::Celerity::FetchValueQuery fetchTransformByObjectId;
    Emergence::Celerity::Transform2dWorldAccessor transformWorldAccessor;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertPrototype;
    Emergence::Celerity::InsertLongTermQuery insertMarker;
    Emergence::Celerity::InsertLongTermQuery insertLayerSetup;
    Emergence::Celerity::InsertLongTermQuery insertTeam;
};

SpawnManager::SpawnManager (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),

      fetchMarkerRemovedEvents (FETCH_SEQUENCE (SpawnMarkerComponentRemovedFixedEvent)),

      editSpawnByObjectId (EDIT_VALUE_1F (SpawnComponent, objectId)),
      editSpawnByAscendingNextSpawnTimeNs (EDIT_ASCENDING_RANGE (SpawnComponent, nextSpawnTimeNs)),
      fetchLayerSetupByObjectId (FETCH_VALUE_1F (LayerSetupComponent, objectId)),
      fetchTeamByObjectId (FETCH_VALUE_1F (TeamComponent, objectId)),

      fetchTransformByObjectId (FETCH_VALUE_1F (Emergence::Celerity::Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertPrototype (INSERT_LONG_TERM (Emergence::Celerity::PrototypeComponent)),
      insertMarker (INSERT_LONG_TERM (SpawnMarkerComponent)),
      insertLayerSetup (INSERT_LONG_TERM (LayerSetupComponent)),
      insertTeam (INSERT_LONG_TERM (TeamComponent))
{
    _constructor.DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::Assembly::Checkpoint::STARTED);
}

void SpawnManager::Execute () noexcept
{
    // We're executing spawn even during loading in order to avoid all-spawns-in-one-frame situation when loading ends.

    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);
    ProcessSpawnedObjectsRemoval (time);
    ProcessScheduledSpawns (time);
}

void SpawnManager::ProcessSpawnedObjectsRemoval (const Emergence::Celerity::TimeSingleton *_time) noexcept
{
    for (auto eventCursor = fetchMarkerRemovedEvents.Execute ();
         const auto *event = static_cast<const SpawnMarkerComponentRemovedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        auto spawnCursor = editSpawnByObjectId.Execute (&event->spawnObjectId);
        if (auto *spawn = static_cast<SpawnComponent *> (*spawnCursor))
        {
            EMERGENCE_ASSERT (spawn->currentSpawnedCount > 0u);
            const uint8_t previousSpawnedCount = spawn->currentSpawnedCount--;

            if (spawn->respawn && spawn->maxSpawnedCount == previousSpawnedCount &&
                spawn->nextSpawnTimeNs == std::numeric_limits<uint64_t>::max ())
            {
                // Schedule next spawn if it is not already scheduled.
                spawn->nextSpawnTimeNs = _time->fixedTimeNs + spawn->spawnDelayNs;
            }
        }
    }
}

void SpawnManager::ProcessScheduledSpawns (const Emergence::Celerity::TimeSingleton *_time) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    for (auto cursor = editSpawnByAscendingNextSpawnTimeNs.Execute (nullptr, &_time->fixedTimeNs);
         auto *spawn = static_cast<SpawnComponent *> (*cursor); ++cursor)
    {
        if (spawn->currentSpawnedCount >= spawn->maxSpawnedCount)
        {
            spawn->nextSpawnTimeNs = std::numeric_limits<uint64_t>::max ();
            continue;
        }

        Emergence::Math::Transform2d spawnTransform;
        if (auto transformCursor = fetchTransformByObjectId.Execute (&spawn->objectId);
            const auto *transform = static_cast<const Emergence::Celerity::Transform2dComponent *> (*transformCursor))
        {
            spawnTransform = transform->GetLogicalWorldTransform (transformWorldAccessor);
        }

        auto transformCursor = insertTransform.Execute ();
        auto *transform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
        transform->SetObjectId (world->GenerateId ());
        transform->SetLogicalLocalTransform (spawnTransform);

        auto prototypeCursor = insertPrototype.Execute ();
        auto *prototype = static_cast<Emergence::Celerity::PrototypeComponent *> (++prototypeCursor);
        prototype->objectId = transform->GetObjectId ();
        prototype->descriptorId = spawn->prototypeId;

        auto markerCursor = insertMarker.Execute ();
        auto *marker = static_cast<SpawnMarkerComponent *> (++markerCursor);
        marker->objectId = transform->GetObjectId ();
        marker->spawnObjectId = spawn->objectId;

        Emergence::Container::Optional<uint16_t> layer;
        if (auto layerSetupCursor = fetchLayerSetupByObjectId.Execute (&spawn->objectId);
            const auto *layerSetup = static_cast<const LayerSetupComponent *> (*layerSetupCursor))
        {
            layer = layerSetup->layer;
        }

        if (layer)
        {
            auto layerSetupCursor = insertLayerSetup.Execute ();
            auto *layerSetup = static_cast<LayerSetupComponent *> (++layerSetupCursor);
            layerSetup->objectId = transform->GetObjectId ();
            layerSetup->layer = layer.value ();
        }

        Emergence::Container::Optional<uint16_t> team;
        if (auto teamCursor = fetchTeamByObjectId.Execute (&spawn->objectId);
            const auto *spawnTeam = static_cast<const TeamComponent *> (*teamCursor))
        {
            team = spawnTeam->teamId;
        }

        if (team)
        {
            auto teamCursor = insertTeam.Execute ();
            auto *spawnedTeam = static_cast<TeamComponent *> (++teamCursor);
            spawnedTeam->objectId = transform->GetObjectId ();
            spawnedTeam->teamId = team.value ();
        }

        ++spawn->currentSpawnedCount;
        if (spawn->currentSpawnedCount == spawn->maxSpawnedCount)
        {
            spawn->nextSpawnTimeNs = std::numeric_limits<uint64_t>::max ();
        }
        else
        {
            spawn->nextSpawnTimeNs = _time->fixedTimeNs + spawn->spawnDelayNs;
        }
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Emergence::Memory::Literals;

    _pipelineBuilder.AddTask ("Spawn::RemoveSpawns"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::TransformNodeCleanupFixedEvent, SpawnComponent, objectId)
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Spawn::RemoveSpawnMarkers"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::TransformNodeCleanupFixedEvent, SpawnMarkerComponent, objectId)
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("Spawn::SpawnManager"_us).SetExecutor<SpawnManager> ();
}
} // namespace Spawn
