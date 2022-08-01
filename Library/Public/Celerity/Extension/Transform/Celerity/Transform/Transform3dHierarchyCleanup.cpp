#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>
#include <Celerity/Transform/Transform3dHierarchyCleanup.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity::HierarchyCleanup
{
const Memory::UniqueString Checkpoint::DETACHMENT_DETECTION_STARTED {
    "Transform3dHierarchyCleanup::DetachmentDetectionStarted"};
const Memory::UniqueString Checkpoint::DETACHMENT_DETECTION_FINISHED {
    "Transform3dHierarchyCleanup::DetachmentDetectionFinished"};

const Memory::UniqueString Checkpoint::DETACHED_REMOVAL_STARTED {"Transform3dHierarchyCleanup::DetachedRemovalStarted"};
const Memory::UniqueString Checkpoint::DETACHED_REMOVAL_FINISHED {
    "Transform3dHierarchyCleanup::DetachedRemovalFinished"};

namespace Tasks
{
static const Memory::UniqueString DETACHMENT_DETECTOR {"Transform3dHierarchyCleanup::DetachmentDetector"};

static const Memory::UniqueString DETACHED_TRANSFORM_REMOVER {"Transform3dHierarchyCleanup::DetachedTransformRemover"};
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
    DetachmentDetector (TaskConstructor &_constructor, const StandardLayout::Mapping &_removalEventType) noexcept;

    void Execute () noexcept;

private:
    void FindDetachedTransforms (UniqueId _parentObjectId) noexcept;

    FetchSequenceQuery fetchTransformRemovedEvents;
    FetchValueQuery fetchTransformByParentObjectId;
    InsertShortTermQuery insertDetachmentMessage;
};

DetachmentDetector::DetachmentDetector (TaskConstructor &_constructor,
                                        const StandardLayout::Mapping &_removalEventType) noexcept
    : fetchTransformRemovedEvents (_constructor.FetchSequence (_removalEventType)),
      fetchTransformByParentObjectId (FETCH_VALUE_1F (Transform3dComponent, parentObjectId)),
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
         const auto *transform = static_cast<const Transform3dComponent *> (*cursor); ++cursor)
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
    DetachedTransformRemover (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    ModifySequenceQuery modifyDetachmentMessages;
    RemoveValueQuery removeTransformById;
};

DetachedTransformRemover::DetachedTransformRemover (TaskConstructor &_constructor) noexcept
    : modifyDetachmentMessages (MODIFY_SEQUENCE (DetachmentMessage)),
      removeTransformById (REMOVE_VALUE_1F (Transform3dComponent, objectId))
{
    _constructor.DependOn (Checkpoint::DETACHED_REMOVAL_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::DETACHED_REMOVAL_FINISHED);
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

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (Tasks::DETACHMENT_DETECTOR)
        .SetExecutor<DetachmentDetector> (Transform3dComponentRemovedFixedEvent::Reflect ().mapping);
    _pipelineBuilder.AddTask (Tasks::DETACHED_TRANSFORM_REMOVER).SetExecutor<DetachedTransformRemover> ();
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (Tasks::DETACHMENT_DETECTOR)
        .SetExecutor<DetachmentDetector> (Transform3dComponentRemovedNormalEvent::Reflect ().mapping);
    _pipelineBuilder.AddTask (Tasks::DETACHED_TRANSFORM_REMOVER).SetExecutor<DetachedTransformRemover> ();
}
} // namespace Emergence::Celerity::HierarchyCleanup
