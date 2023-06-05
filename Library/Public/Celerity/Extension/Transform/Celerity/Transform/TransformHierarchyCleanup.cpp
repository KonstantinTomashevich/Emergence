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

static const Memory::UniqueString TRANSFORM_CLEANER {"TransformHierarchyCleanup::FixedTransformCleaner"};
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

#define SCHEDULERS(Dimensions)                                                                                         \
    class FixedCleanup##Dimensions##dScheduler final : public TaskExecutorBase<FixedCleanup##Dimensions##dScheduler>   \
    {                                                                                                                  \
    public:                                                                                                            \
        FixedCleanup##Dimensions##dScheduler (TaskConstructor &_constructor) noexcept;                                 \
                                                                                                                       \
        void Execute () noexcept;                                                                                      \
                                                                                                                       \
    private:                                                                                                           \
        void ScheduleCleanup (InsertShortTermQuery::Cursor &_insertionCursor, UniqueId _objectId) noexcept;            \
                                                                                                                       \
        FetchSequenceQuery fetchRemovalEvents;                                                                         \
        FetchValueQuery fetchComponentByParent;                                                                        \
        RemoveValueQuery removeTransformCleanedUpMarker;                                                               \
        InsertShortTermQuery insertCleanupEvent;                                                                       \
    };                                                                                                                 \
                                                                                                                       \
    FixedCleanup##Dimensions##dScheduler::FixedCleanup##Dimensions##dScheduler (                                       \
        TaskConstructor &_constructor) noexcept                                                                        \
        : TaskExecutorBase (_constructor),                                                                             \
                                                                                                                       \
          fetchRemovalEvents (FETCH_SEQUENCE (Transform##Dimensions##dComponentRemovedFixedEvent)),                    \
          fetchComponentByParent (FETCH_VALUE_1F (Transform##Dimensions##dComponent, parentObjectId)),                 \
          removeTransformCleanedUpMarker (REMOVE_VALUE_1F (TransformCleanedUpMarkerComponent, objectId)),              \
          insertCleanupEvent (INSERT_SHORT_TERM (TransformNodeCleanupFixedEvent))                                      \
    {                                                                                                                  \
        _constructor.DependOn (Checkpoint::STARTED);                                                                   \
        _constructor.MakeDependencyOf (Checkpoint::CLEANUP_STARTED);                                                   \
    }                                                                                                                  \
                                                                                                                       \
    void FixedCleanup##Dimensions##dScheduler::Execute () noexcept                                                     \
    {                                                                                                                  \
        auto insertionCursor = insertCleanupEvent.Execute ();                                                          \
        for (auto removalEventCursor = fetchRemovalEvents.Execute ();                                                  \
             const auto *removedId = static_cast<const UniqueId *> (*removalEventCursor); ++removalEventCursor)        \
        {                                                                                                              \
            ScheduleCleanup (insertionCursor, *removedId);                                                             \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    void FixedCleanup##Dimensions##dScheduler::ScheduleCleanup (InsertShortTermQuery::Cursor &_insertionCursor,        \
                                                                UniqueId _objectId) noexcept                           \
    {                                                                                                                  \
        if (auto cursor = removeTransformCleanedUpMarker.Execute (&_objectId); cursor.ReadConst ())                    \
        {                                                                                                              \
            /* Transform was removed by cleanup logic, therefore cleanup already happened. */                          \
            ~cursor;                                                                                                   \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        *static_cast<TransformNodeCleanupFixedEvent *> (++_insertionCursor) = {_objectId};                             \
        for (auto childCursor = fetchComponentByParent.Execute (&_objectId);                                           \
             const auto *component = static_cast<const Transform##Dimensions##dComponent *> (*childCursor);            \
             ++childCursor)                                                                                            \
        {                                                                                                              \
            ScheduleCleanup (_insertionCursor, component->GetObjectId ());                                             \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    class NormalCleanup##Dimensions##dScheduler final : public TaskExecutorBase<NormalCleanup##Dimensions##dScheduler> \
    {                                                                                                                  \
    public:                                                                                                            \
        NormalCleanup##Dimensions##dScheduler (TaskConstructor &_constructor) noexcept;                                \
                                                                                                                       \
        void Execute () noexcept;                                                                                      \
                                                                                                                       \
    private:                                                                                                           \
        void ScheduleCleanup (InsertShortTermQuery::Cursor &_insertionCursor,                                          \
                              UniqueId _objectId,                                                                      \
                              bool _cleanupTransform) noexcept;                                                        \
                                                                                                                       \
        FetchSequenceQuery fetchFixedRemovalEvents;                                                                    \
        FetchSequenceQuery fetchNormalRemovalEvents;                                                                   \
        FetchValueQuery fetchComponentByParent;                                                                        \
        RemoveValueQuery removeTransformCleanedUpMarker;                                                               \
        InsertShortTermQuery insertCleanupEvent;                                                                       \
    };                                                                                                                 \
                                                                                                                       \
    NormalCleanup##Dimensions##dScheduler::NormalCleanup##Dimensions##dScheduler (                                     \
        TaskConstructor &_constructor) noexcept                                                                        \
        : TaskExecutorBase (_constructor),                                                                             \
                                                                                                                       \
          fetchFixedRemovalEvents (FETCH_SEQUENCE (Transform##Dimensions##dComponentRemovedFixedToNormalEvent)),       \
          fetchNormalRemovalEvents (FETCH_SEQUENCE (Transform##Dimensions##dComponentRemovedNormalEvent)),             \
          fetchComponentByParent (FETCH_VALUE_1F (Transform##Dimensions##dComponent, parentObjectId)),                 \
          removeTransformCleanedUpMarker (REMOVE_VALUE_1F (TransformCleanedUpMarkerComponent, objectId)),              \
          insertCleanupEvent (INSERT_SHORT_TERM (TransformNodeCleanupNormalEvent))                                     \
    {                                                                                                                  \
        _constructor.DependOn (Checkpoint::STARTED);                                                                   \
        _constructor.MakeDependencyOf (Checkpoint::CLEANUP_STARTED);                                                   \
    }                                                                                                                  \
                                                                                                                       \
    void NormalCleanup##Dimensions##dScheduler::Execute () noexcept                                                    \
    {                                                                                                                  \
        auto insertionCursor = insertCleanupEvent.Execute ();                                                          \
        for (auto removalEventCursor = fetchFixedRemovalEvents.Execute ();                                             \
             const auto *removedId = static_cast<const UniqueId *> (*removalEventCursor); ++removalEventCursor)        \
        {                                                                                                              \
            ScheduleCleanup (insertionCursor, *removedId, false);                                                      \
        }                                                                                                              \
                                                                                                                       \
        for (auto removalEventCursor = fetchNormalRemovalEvents.Execute ();                                            \
             const auto *removedId = static_cast<const UniqueId *> (*removalEventCursor); ++removalEventCursor)        \
        {                                                                                                              \
            ScheduleCleanup (insertionCursor, *removedId, true);                                                       \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    void NormalCleanup##Dimensions##dScheduler::ScheduleCleanup (InsertShortTermQuery::Cursor &_insertionCursor,       \
                                                                 UniqueId _objectId, bool _cleanupTransform) noexcept  \
    {                                                                                                                  \
        if (auto cursor = removeTransformCleanedUpMarker.Execute (&_objectId); cursor.ReadConst ())                    \
        {                                                                                                              \
            /* Transform was removed by cleanup logic, therefore cleanup already happened. */                          \
            ~cursor;                                                                                                   \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        *static_cast<TransformNodeCleanupNormalEvent *> (++_insertionCursor) = {_objectId, _cleanupTransform};         \
        for (auto childCursor = fetchComponentByParent.Execute (&_objectId);                                           \
             const auto *component = static_cast<const Transform##Dimensions##dComponent *> (*childCursor);            \
             ++childCursor)                                                                                            \
        {                                                                                                              \
            ScheduleCleanup (_insertionCursor, component->GetObjectId (), _cleanupTransform);                          \
        }                                                                                                              \
    }

SCHEDULERS (2)
SCHEDULERS (3)

class FixedTransformCleaner final : public TaskExecutorBase<FixedTransformCleaner>
{
public:
    FixedTransformCleaner (TaskConstructor &_constructor,
                           const StandardLayout::Mapping &_componentMapping,
                           StandardLayout::FieldId _idField) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchCleanupEvents;
    RemoveValueQuery removeComponentById;
    InsertLongTermQuery insertTransformCleanedUpMarker;
};

FixedTransformCleaner::FixedTransformCleaner (TaskConstructor &_constructor,
                                              const StandardLayout::Mapping &_componentMapping,
                                              StandardLayout::FieldId _idField) noexcept
    : TaskExecutorBase (_constructor),

      fetchCleanupEvents (FETCH_SEQUENCE (TransformNodeCleanupFixedEvent)),
      removeComponentById (_constructor.RemoveValue (_componentMapping, {_idField})),
      insertTransformCleanedUpMarker (INSERT_LONG_TERM (TransformCleanedUpMarkerComponent))
{
    _constructor.DependOn (Checkpoint::CLEANUP_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void FixedTransformCleaner::Execute () noexcept
{
    auto markerCursor = insertTransformCleanedUpMarker.Execute ();
    for (auto eventCursor = fetchCleanupEvents.Execute ();
         const auto *event = static_cast<const TransformNodeCleanupFixedEvent *> (*eventCursor); ++eventCursor)
    {
        if (auto removalCursor = removeComponentById.Execute (&event->objectId); removalCursor.ReadConst ())
        {
            ~removalCursor;
            static_cast<TransformCleanedUpMarkerComponent *> (++markerCursor)->objectId = event->objectId;
        }
    }
}

class NormalTransformCleaner final : public TaskExecutorBase<NormalTransformCleaner>
{
public:
    NormalTransformCleaner (TaskConstructor &_constructor,
                            const StandardLayout::Mapping &_componentMapping,
                            StandardLayout::FieldId _idField) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchCleanupEvents;
    RemoveValueQuery removeComponentById;
    InsertLongTermQuery insertTransformCleanedUpMarker;
};

NormalTransformCleaner::NormalTransformCleaner (TaskConstructor &_constructor,
                                                const StandardLayout::Mapping &_componentMapping,
                                                StandardLayout::FieldId _idField) noexcept
    : TaskExecutorBase (_constructor),

      fetchCleanupEvents (FETCH_SEQUENCE (TransformNodeCleanupNormalEvent)),
      removeComponentById (_constructor.RemoveValue (_componentMapping, {_idField})),
      insertTransformCleanedUpMarker (INSERT_LONG_TERM (TransformCleanedUpMarkerComponent))
{
    _constructor.DependOn (Checkpoint::CLEANUP_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void NormalTransformCleaner::Execute () noexcept
{
    auto markerCursor = insertTransformCleanedUpMarker.Execute ();
    for (auto eventCursor = fetchCleanupEvents.Execute ();
         const auto *event = static_cast<const TransformNodeCleanupNormalEvent *> (*eventCursor); ++eventCursor)
    {
        if (event->cleanupTransform)
        {
            if (auto removalCursor = removeComponentById.Execute (&event->objectId); removalCursor.ReadConst ())
            {
                ~removalCursor;
                static_cast<TransformCleanedUpMarkerComponent *> (++markerCursor)->objectId = event->objectId;
            }
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

    _pipelineBuilder.AddTask (Tasks::CLEANUP_SCHEDULER).SetExecutor<FixedCleanup2dScheduler> ();
    _pipelineBuilder.AddTask (Tasks::TRANSFORM_CLEANER)
        .SetExecutor<FixedTransformCleaner> (Transform2dComponent::Reflect ().mapping,
                                             Transform2dComponent::Reflect ().objectId);
}

void Add2dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);

    _pipelineBuilder.AddTask (Tasks::CLEANUP_SCHEDULER).SetExecutor<NormalCleanup2dScheduler> ();
    _pipelineBuilder.AddTask (Tasks::TRANSFORM_CLEANER)
        .SetExecutor<NormalTransformCleaner> (Transform2dComponent::Reflect ().mapping,
                                              Transform2dComponent::Reflect ().objectId);
}

void Add3dToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);

    _pipelineBuilder.AddTask (Tasks::CLEANUP_SCHEDULER).SetExecutor<FixedCleanup3dScheduler> ();
    _pipelineBuilder.AddTask (Tasks::TRANSFORM_CLEANER)
        .SetExecutor<FixedTransformCleaner> (Transform3dComponent::Reflect ().mapping,
                                             Transform3dComponent::Reflect ().objectId);
}

void Add3dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);

    _pipelineBuilder.AddTask (Tasks::CLEANUP_SCHEDULER).SetExecutor<NormalCleanup3dScheduler> ();

    _pipelineBuilder.AddTask (Tasks::TRANSFORM_CLEANER)
        .SetExecutor<NormalTransformCleaner> (Transform3dComponent::Reflect ().mapping,
                                              Transform3dComponent::Reflect ().objectId);
}
} // namespace Emergence::Celerity::TransformHierarchyCleanup
