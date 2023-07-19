#include <API/Common/BlockCast.hpp>

#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>

#include <Math/Scalar.hpp>

namespace Emergence::Celerity
{
template <typename Transform>
Transform InterpolateTransform (const Transform &_source, const Transform &_target, float _progress);

template <>
Math::Transform2d InterpolateTransform<Math::Transform2d> (const Math::Transform2d &_source,
                                                           const Math::Transform2d &_target,
                                                           float _progress)
{
    return {Math::Lerp (_source.translation, _target.translation, _progress),
            Math::Lerp (_source.rotation, _target.rotation, _progress),
            Math::Lerp (_source.scale, _target.scale, _progress)};
}

template <>
Math::Transform3d InterpolateTransform<Math::Transform3d> (const Math::Transform3d &_source,
                                                           const Math::Transform3d &_target,
                                                           float _progress)
{
    return {Math::Lerp (_source.translation, _target.translation, _progress),
            Math::SLerp (_source.rotation, _target.rotation, _progress),
            Math::Lerp (_source.scale, _target.scale, _progress)};
}

template <typename TransformComponentType>
class TransformVisualSynchronizer final : public TaskExecutorBase<TransformVisualSynchronizer<TransformComponentType>>
{
public:
    TransformVisualSynchronizer (TaskConstructor &_constructor,
                                 const StandardLayout::Mapping &_addedEventType) noexcept;

    void Execute () noexcept;

private:
    FetchSingletonQuery fetchTime;
    FetchSequenceQuery fetchTransformAddedFromFixedEvents;
    EditValueQuery editTransformById;
    EditSignalQuery editTransformsWithUpdateFlag;
};

template <typename TransformComponentType>
TransformVisualSynchronizer<TransformComponentType>::TransformVisualSynchronizer (
    TaskConstructor &_constructor, const StandardLayout::Mapping &_addedEventType) noexcept
    : TaskExecutorBase<TransformVisualSynchronizer> (_constructor),

      fetchTime (FETCH_SINGLETON (TimeSingleton)),
      fetchTransformAddedFromFixedEvents (_constructor.FetchSequence (_addedEventType)),
      editTransformById (EDIT_VALUE_1F (TransformComponentType, objectId)),
      editTransformsWithUpdateFlag (EDIT_SIGNAL (TransformComponentType, visualTransformSyncNeeded, true))
{
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.DependOn (TransformVisualSync::Checkpoint::STARTED);
    _constructor.MakeDependencyOf (TransformVisualSync::Checkpoint::FINISHED);
}

template <typename TransformComponentType>
void TransformVisualSynchronizer<TransformComponentType>::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const TimeSingleton *> (*timeCursor);

    // We need to initialize visual transforms for logical transforms that were initialized using reflection.
    // Reflection-based loaders directly edit transforms and therefore usual sync after SetLogicalLocalTransform
    // is never scheduled. This results in outdated visual transform that is never synced unless
    // object logical transform changes. To solve this issue we detect transforms that were added during
    // fixed update and set their visual transform manually unless sync is already requested.
    for (auto eventCursor = fetchTransformAddedFromFixedEvents.Execute ();
         const auto *event = static_cast<const Transform2dComponentAddedFixedToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto cursor = editTransformById.Execute (&event->objectId);
        if (auto *transform = static_cast<TransformComponentType *> (*cursor);
            transform && !transform->visualTransformSyncNeeded)
        {
            transform->SetVisualLocalTransform (transform->logicalLocalTransform);
        }
    }

    auto transformCursor = editTransformsWithUpdateFlag.Execute ();
    while (auto *transform = static_cast<TransformComponentType *> (*transformCursor))
    {
        if (transform->lastObservedLogicalTransformRevision != transform->logicalLocalTransformRevision)
        {
            transform->lastObservedLogicalTransformRevision = transform->logicalLocalTransformRevision;
            transform->logicalTransformLastObservationTimeNs = time->fixedTimeNs;
        }

        // If we never interpolated this object before or last interpolation was
        // interrupted,  fix last sync time stamp so elapsed time will be zero.
        if (transform->visualTransformLastSyncTimeNs == 0u)
        {
            transform->visualTransformLastSyncTimeNs = time->normalTimeNs;
        }

        // Apply interpolation.
        const std::uint64_t elapsed = time->normalTimeNs - transform->visualTransformLastSyncTimeNs;

        const std::uint64_t duration = std::max (std::uint64_t {1u}, transform->logicalTransformLastObservationTimeNs -
                                                                         transform->visualTransformLastSyncTimeNs);

        const float progress = Math::Clamp (static_cast<float> (elapsed) / static_cast<float> (duration), 0.0f, 1.0f);

        const auto &source = transform->GetVisualLocalTransform ();
        const auto &target = transform->GetLogicalLocalTransform ();
        transform->SetVisualLocalTransform (InterpolateTransform (source, target, progress));

        transform->visualTransformLastSyncTimeNs = time->normalTimeNs;
        transform->visualTransformSyncNeeded =
            transform->visualTransformLastSyncTimeNs < transform->logicalTransformLastObservationTimeNs;
        ++transformCursor;
    }
}

namespace TransformVisualSync
{
const Memory::UniqueString Checkpoint::STARTED {"TransformVisualSyncStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"TransformVisualSyncFinished"};

void Add2dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformVisualSync");
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Memory::UniqueString {"Transform2dVisualSync"})
        .SetExecutor<TransformVisualSynchronizer<Transform2dComponent>> (
            Transform2dComponentAddedFixedToNormalEvent::Reflect ().mapping);
}

void Add3dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TransformVisualSync");
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Memory::UniqueString {"Transform3dVisualSync"})
        .SetExecutor<TransformVisualSynchronizer<Transform3dComponent>> (
            Transform3dComponentAddedFixedToNormalEvent::Reflect ().mapping);
}
} // namespace TransformVisualSync
} // namespace Emergence::Celerity
