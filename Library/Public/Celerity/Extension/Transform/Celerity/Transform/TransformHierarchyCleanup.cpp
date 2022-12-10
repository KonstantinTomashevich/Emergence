#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity::TransformHierarchyCleanup
{
const Memory::UniqueString Checkpoint::STARTED {"TransformHierarchyCleanup::Started"};
const Memory::UniqueString Checkpoint::CLEANUP_STARTED {"TransformHierarchyCleanup::CleanupStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"TransformHierarchyCleanup::Finished"};

namespace Tasks
{
static const Memory::UniqueString CLEANUP_SCHEDULER {"TransformHierarchyCleanup::CleanupScheduler"};

static const Memory::UniqueString TRANSFORM_CLEANER {"TransformHierarchyCleanup::TransformCleaner"};
} // namespace Tasks

struct TransformCleanedUpMarkerComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const TransformCleanedUpMarkerComponent::Reflection &TransformCleanedUpMarkerComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TransformCleanedUpMarkerComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

class CleanupScheduler final : public TaskExecutorBase<CleanupScheduler>
{
public:
    CleanupScheduler (TaskConstructor &_constructor,
                      const StandardLayout::Mapping &_componentMapping,
                      StandardLayout::FieldId _idField,
                      StandardLayout::FieldId _parentField,
                      const StandardLayout::Mapping &_removalEventMapping,
                      const Container::Vector<StandardLayout::Mapping> &_cleanupEventMappings) noexcept;

    void Execute () noexcept;

private:
    void ScheduleCleanup (UniqueId _objectId) noexcept;

    FetchSequenceQuery fetchRemovalEvents;
    FetchValueQuery fetchComponentByParent;
    RemoveValueQuery removeTransformCleanedUpMarker;
    Container::Vector<InsertShortTermQuery> insertCleanupEvent {Memory::Profiler::AllocationGroup::Top ()};
    StandardLayout::Field componentObjectIdField;
};

CleanupScheduler::CleanupScheduler (TaskConstructor &_constructor,
                                    const StandardLayout::Mapping &_componentMapping,
                                    StandardLayout::FieldId _idField,
                                    StandardLayout::FieldId _parentField,
                                    const StandardLayout::Mapping &_removalEventMapping,
                                    const Container::Vector<StandardLayout::Mapping> &_cleanupEventMappings) noexcept
    : fetchRemovalEvents (_constructor.FetchSequence (_removalEventMapping)),
      fetchComponentByParent (_constructor.FetchValue (_componentMapping, {_parentField})),
      removeTransformCleanedUpMarker (REMOVE_VALUE_1F (TransformCleanedUpMarkerComponent, objectId)),
      componentObjectIdField (_componentMapping.GetField (_idField))
{
    insertCleanupEvent.reserve (_cleanupEventMappings.size ());
    for (const StandardLayout::Mapping &mapping : _cleanupEventMappings)
    {
        insertCleanupEvent.emplace_back (_constructor.InsertShortTerm (mapping));
    }

    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::CLEANUP_STARTED);
}

void CleanupScheduler::Execute () noexcept
{
    for (auto removalEventCursor = fetchRemovalEvents.Execute ();
         const auto *removedId = static_cast<const UniqueId *> (*removalEventCursor); ++removalEventCursor)
    {
        ScheduleCleanup (*removedId);
    }
}

void CleanupScheduler::ScheduleCleanup (UniqueId _objectId) noexcept
{
    if (auto cursor = removeTransformCleanedUpMarker.Execute (&_objectId); cursor.ReadConst ())
    {
        // Transform was removed by cleanup logic, therefore cleanup already happened.
        ~cursor;
        return;
    }

    for (auto &query : insertCleanupEvent)
    {
        auto cursor = query.Execute ();
        *static_cast<UniqueId *> (++cursor) = _objectId;
    }

    for (auto childCursor = fetchComponentByParent.Execute (&_objectId); const void *component = *childCursor;
         ++childCursor)
    {
        ScheduleCleanup (*static_cast<const UniqueId *> (componentObjectIdField.GetValue (component)));
    }
}

class TransformCleaner final : public TaskExecutorBase<TransformCleaner>
{
public:
    TransformCleaner (TaskConstructor &_constructor,
                      const StandardLayout::Mapping &_componentMapping,
                      StandardLayout::FieldId _idField,
                      const StandardLayout::Mapping &_cleanupEventMapping) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchCleanupEvents;
    RemoveValueQuery removeComponentById;
    InsertLongTermQuery insertTransformCleanedUpMarker;
};

TransformCleaner::TransformCleaner (TaskConstructor &_constructor,
                                    const StandardLayout::Mapping &_componentMapping,
                                    StandardLayout::FieldId _idField,
                                    const StandardLayout::Mapping &_cleanupEventMapping) noexcept
    : fetchCleanupEvents (_constructor.FetchSequence (_cleanupEventMapping)),
      removeComponentById (_constructor.RemoveValue (_componentMapping, {_idField})),
      insertTransformCleanedUpMarker (INSERT_LONG_TERM (TransformCleanedUpMarkerComponent))
{
    _constructor.DependOn (Checkpoint::CLEANUP_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void TransformCleaner::Execute () noexcept
{
    auto markerCursor = insertTransformCleanedUpMarker.Execute ();
    for (auto eventCursor = fetchCleanupEvents.Execute ();
         const auto *cleanupId = static_cast<const UniqueId *> (*eventCursor); ++eventCursor)
    {
        if (auto removalCursor = removeComponentById.Execute (cleanupId); removalCursor.ReadConst ())
        {
            ~removalCursor;
            static_cast<TransformCleanedUpMarkerComponent *> (++markerCursor)->objectId = *cleanupId;
        }
    }
}

static void AddCheckpoints (PipelineBuilder &_pipelineBuilder)
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::CLEANUP_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
}

void Add2dToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);

    _pipelineBuilder.AddTask (Tasks::CLEANUP_SCHEDULER)
        .SetExecutor<CleanupScheduler> (
            Transform2dComponent::Reflect ().mapping, Transform2dComponent::Reflect ().objectId,
            Transform2dComponent::Reflect ().parentObjectId, Transform2dComponentRemovedFixedEvent::Reflect ().mapping,
            Container::Vector<StandardLayout::Mapping> {
                TransformNodeCleanupFixedEvent::Reflect ().mapping,
                TransformNodeCleanupFixedToNormalEvent::Reflect ().mapping,
            });

    _pipelineBuilder.AddTask (Tasks::TRANSFORM_CLEANER)
        .SetExecutor<TransformCleaner> (Transform2dComponent::Reflect ().mapping,
                                        Transform2dComponent::Reflect ().objectId,
                                        TransformNodeCleanupFixedEvent::Reflect ().mapping);
}

void Add2dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);

    _pipelineBuilder.AddTask (Tasks::CLEANUP_SCHEDULER)
        .SetExecutor<CleanupScheduler> (
            Transform2dComponent::Reflect ().mapping, Transform2dComponent::Reflect ().objectId,
            Transform2dComponent::Reflect ().parentObjectId, Transform2dComponentRemovedNormalEvent::Reflect ().mapping,
            Container::Vector<StandardLayout::Mapping> {
                TransformNodeCleanupNormalEvent::Reflect ().mapping,
            });

    _pipelineBuilder.AddTask (Tasks::TRANSFORM_CLEANER)
        .SetExecutor<TransformCleaner> (Transform2dComponent::Reflect ().mapping,
                                        Transform2dComponent::Reflect ().objectId,
                                        TransformNodeCleanupNormalEvent::Reflect ().mapping);
}

void Add3dToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);
    
    _pipelineBuilder.AddTask (Tasks::CLEANUP_SCHEDULER)
        .SetExecutor<CleanupScheduler> (
            Transform3dComponent::Reflect ().mapping, Transform3dComponent::Reflect ().objectId,
            Transform3dComponent::Reflect ().parentObjectId, Transform3dComponentRemovedFixedEvent::Reflect ().mapping,
            Container::Vector<StandardLayout::Mapping> {
                TransformNodeCleanupFixedEvent::Reflect ().mapping,
                TransformNodeCleanupFixedToNormalEvent::Reflect ().mapping,
            });

    _pipelineBuilder.AddTask (Tasks::TRANSFORM_CLEANER)
        .SetExecutor<TransformCleaner> (Transform3dComponent::Reflect ().mapping,
                                        Transform3dComponent::Reflect ().objectId,
                                        TransformNodeCleanupFixedEvent::Reflect ().mapping);
}

void Add3dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);

    _pipelineBuilder.AddTask (Tasks::CLEANUP_SCHEDULER)
        .SetExecutor<CleanupScheduler> (
            Transform3dComponent::Reflect ().mapping, Transform3dComponent::Reflect ().objectId,
            Transform3dComponent::Reflect ().parentObjectId, Transform3dComponentRemovedNormalEvent::Reflect ().mapping,
            Container::Vector<StandardLayout::Mapping> {
                TransformNodeCleanupNormalEvent::Reflect ().mapping,
            });

    _pipelineBuilder.AddTask (Tasks::TRANSFORM_CLEANER)
        .SetExecutor<TransformCleaner> (Transform3dComponent::Reflect ().mapping,
                                        Transform3dComponent::Reflect ().objectId,
                                        TransformNodeCleanupNormalEvent::Reflect ().mapping);
}
} // namespace Emergence::Celerity::TransformHierarchyCleanup
