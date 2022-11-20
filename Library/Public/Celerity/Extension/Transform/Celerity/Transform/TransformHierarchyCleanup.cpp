#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity::TransformHierarchyCleanup
{
const Memory::UniqueString Checkpoint::DETACHED_REMOVAL_STARTED {"TransformHierarchyCleanup::DetachedRemovalStarted"};
const Memory::UniqueString Checkpoint::DETACHED_REMOVAL_FINISHED {"TransformHierarchyCleanup::DetachedRemovalFinished"};

const Memory::UniqueString Checkpoint::DETACHMENT_DETECTION_STARTED {
    "TransformHierarchyCleanup::DetachmentDetectionStarted"};
const Memory::UniqueString Checkpoint::DETACHMENT_DETECTION_FINISHED {
    "TransformHierarchyCleanup::DetachmentDetectionFinished"};

namespace Tasks
{
static const Memory::UniqueString DETACHMENT_DETECTOR {"TransformHierarchyCleanup::DetachmentDetector"};

static const Memory::UniqueString DETACHED_TRANSFORM_REMOVER {"TransformHierarchyCleanup::DetachedTransformRemover"};
} // namespace Tasks

struct DetachmentMessage final
{
    UniqueId transformId;

    struct Reflection final
    {
        StandardLayout::FieldId transformId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const DetachmentMessage::Reflection &DetachmentMessage::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DetachmentMessage);
        EMERGENCE_MAPPING_REGISTER_REGULAR (transformId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

class DetachmentDetector final : public TaskExecutorBase<DetachmentDetector>
{
public:
    DetachmentDetector (TaskConstructor &_constructor,
                        const StandardLayout::Mapping &_transformType,
                        StandardLayout::FieldId _parentObjectIdField,
                        const StandardLayout::Mapping &_removalEventType) noexcept;

    void Execute () noexcept;

private:
    void FindDetachedTransforms (UniqueId _parentObjectId) noexcept;

    FetchSequenceQuery fetchTransformRemovedEvents;
    FetchValueQuery fetchTransformByParentObjectId;
    InsertShortTermQuery insertDetachmentMessage;
};

DetachmentDetector::DetachmentDetector (TaskConstructor &_constructor,
                                        const StandardLayout::Mapping &_transformType,
                                        StandardLayout::FieldId _parentObjectIdField,
                                        const StandardLayout::Mapping &_removalEventType) noexcept
    : fetchTransformRemovedEvents (_constructor.FetchSequence (_removalEventType)),
      fetchTransformByParentObjectId (_constructor.FetchValue (_transformType, {_parentObjectIdField})),
      insertDetachmentMessage (INSERT_SHORT_TERM (DetachmentMessage))
{
    _constructor.DependOn (Checkpoint::DETACHMENT_DETECTION_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::DETACHMENT_DETECTION_FINISHED);
}

void DetachmentDetector::Execute () noexcept
{
    for (auto eventCursor = fetchTransformRemovedEvents.Execute (); const void *event = *eventCursor; ++eventCursor)
    {
        FindDetachedTransforms (*static_cast<const UniqueId *> (event));
    }
}

void DetachmentDetector::FindDetachedTransforms (UniqueId _parentObjectId) noexcept
{
    for (auto cursor = fetchTransformByParentObjectId.Execute (&_parentObjectId);
         const auto *transform = static_cast<const Transform2dComponent *> (*cursor); ++cursor)
    {
        FindDetachedTransforms (transform->GetObjectId ());
        auto messageCursor = insertDetachmentMessage.Execute ();
        auto *message = static_cast<DetachmentMessage *> (++messageCursor);
        message->transformId = transform->GetObjectId ();
    }
}

class DetachedTransformRemover final : public TaskExecutorBase<DetachedTransformRemover>
{
public:
    DetachedTransformRemover (TaskConstructor &_constructor,
                              const StandardLayout::Mapping &_transformType,
                              StandardLayout::FieldId _objectIdField) noexcept;

    void Execute () noexcept;

private:
    ModifySequenceQuery modifyDetachmentMessages;
    RemoveValueQuery removeTransformById;
};

DetachedTransformRemover::DetachedTransformRemover (TaskConstructor &_constructor,
                                                    const StandardLayout::Mapping &_transformType,
                                                    StandardLayout::FieldId _objectIdField) noexcept
    : modifyDetachmentMessages (MODIFY_SEQUENCE (DetachmentMessage)),
      removeTransformById (_constructor.RemoveValue (_transformType, {_objectIdField}))
{
    _constructor.DependOn (Checkpoint::DETACHED_REMOVAL_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::DETACHED_REMOVAL_FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::DETACHMENT_DETECTION_STARTED);
}

void DetachedTransformRemover::Execute () noexcept
{
    for (auto messageCursor = modifyDetachmentMessages.Execute ();
         auto *message = static_cast<DetachmentMessage *> (*messageCursor); ~messageCursor)
    {
        auto transformCursor = removeTransformById.Execute (&message->transformId);
        if (transformCursor.ReadConst ())
        {
            ~transformCursor;
        }
    }
}

static void AddCheckpoints (PipelineBuilder &_pipelineBuilder)
{
    _pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::DETACHMENT_DETECTION_STARTED);
    _pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::DETACHMENT_DETECTION_FINISHED);

    _pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::DETACHED_REMOVAL_STARTED);
    _pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::DETACHED_REMOVAL_FINISHED);
}

void Add2dToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask (Tasks::DETACHMENT_DETECTOR)
        .SetExecutor<DetachmentDetector> (Transform2dComponent::Reflect ().mapping,
                                          Transform2dComponent::Reflect ().parentObjectId,
                                          Transform2dComponentRemovedFixedEvent::Reflect ().mapping);

    _pipelineBuilder.AddTask (Tasks::DETACHED_TRANSFORM_REMOVER)
        .SetExecutor<DetachedTransformRemover> (Transform2dComponent::Reflect ().mapping,
                                                Transform2dComponent::Reflect ().objectId);
}

void Add2dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask (Tasks::DETACHMENT_DETECTOR)
        .SetExecutor<DetachmentDetector> (Transform2dComponent::Reflect ().mapping,
                                          Transform2dComponent::Reflect ().parentObjectId,
                                          Transform2dComponentRemovedNormalEvent::Reflect ().mapping);

    _pipelineBuilder.AddTask (Tasks::DETACHED_TRANSFORM_REMOVER)
        .SetExecutor<DetachedTransformRemover> (Transform2dComponent::Reflect ().mapping,
                                                Transform2dComponent::Reflect ().objectId);
}

void Add3dToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask (Tasks::DETACHMENT_DETECTOR)
        .SetExecutor<DetachmentDetector> (Transform3dComponent::Reflect ().mapping,
                                          Transform3dComponent::Reflect ().parentObjectId,
                                          Transform3dComponentRemovedFixedEvent::Reflect ().mapping);

    _pipelineBuilder.AddTask (Tasks::DETACHED_TRANSFORM_REMOVER)
        .SetExecutor<DetachedTransformRemover> (Transform3dComponent::Reflect ().mapping,
                                                Transform3dComponent::Reflect ().objectId);
}

void Add3dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformHierarchyCleanup");
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask (Tasks::DETACHMENT_DETECTOR)
        .SetExecutor<DetachmentDetector> (Transform3dComponent::Reflect ().mapping,
                                          Transform3dComponent::Reflect ().parentObjectId,
                                          Transform3dComponentRemovedNormalEvent::Reflect ().mapping);

    _pipelineBuilder.AddTask (Tasks::DETACHED_TRANSFORM_REMOVER)
        .SetExecutor<DetachedTransformRemover> (Transform3dComponent::Reflect ().mapping,
                                                Transform3dComponent::Reflect ().objectId);
}
} // namespace Emergence::Celerity::TransformHierarchyCleanup
