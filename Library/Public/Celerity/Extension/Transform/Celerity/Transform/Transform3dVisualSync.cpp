#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>
#include <Celerity/Transform/Transform3dVisualSync.hpp>

#include <Math/Scalar.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Celerity
{
class Transform3dVisualSynchronizer final : public TaskExecutorBase<Transform3dVisualSynchronizer>
{
public:
    Transform3dVisualSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    FetchSingletonQuery fetchTime;
    FetchSequenceQuery fetchTransformAddedFromFixedEvents;
    EditValueQuery editTransformById;
    EditSignalQuery editTransformsWithUpdateFlag;
};

Transform3dVisualSynchronizer::Transform3dVisualSynchronizer (TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (TimeSingleton)),
      fetchTransformAddedFromFixedEvents (FETCH_SEQUENCE (Transform3dComponentAddedFixedToNormalEvent)),
      editTransformById (EDIT_VALUE_1F (Transform3dComponent, objectId)),
      editTransformsWithUpdateFlag (EDIT_SIGNAL (Transform3dComponent, visualTransformSyncNeeded, true))
{
    _constructor.MakeDependencyOf (VisualTransformSync::Checkpoint::SYNC_FINISHED);
}

void Transform3dVisualSynchronizer::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const TimeSingleton *> (*timeCursor);

    // We need to initialize visual transforms for logical transforms that were initialized using reflection.
    // Reflection-based loaders directly edit transforms and therefore usual sync after SetLogicalLocalTransform
    // is never scheduled. This results in outdated visual transform that is never synced unless
    // object logical transform changes. To solve this issue we detect transforms that were added during
    // fixed update and set their visual transform manually unless sync is already requested.
    for (auto eventCursor = fetchTransformAddedFromFixedEvents.Execute ();
         const auto *event = static_cast<const Transform3dComponentAddedFixedToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto cursor = editTransformById.Execute (&event->objectId);
        if (auto *transform = static_cast<Transform3dComponent *> (*cursor);
            transform && !transform->visualTransformSyncNeeded)
        {
            transform->visualLocalTransform = transform->logicalLocalTransform;
        }
    }

    auto transformCursor = editTransformsWithUpdateFlag.Execute ();
    while (auto *transform = static_cast<Transform3dComponent *> (*transformCursor))
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
        const uint64_t elapsed = time->normalTimeNs - transform->visualTransformLastSyncTimeNs;

        const uint64_t duration =
            transform->logicalTransformLastObservationTimeNs - transform->visualTransformLastSyncTimeNs;

        const float progress = Math::Clamp (static_cast<float> (elapsed) / static_cast<float> (duration), 0.0f, 1.0f);

        const Math::Transform3d &source = transform->GetVisualLocalTransform ();
        const Math::Transform3d &target = transform->GetLogicalLocalTransform ();

        transform->SetVisualLocalTransform ({Math::Lerp (source.translation, target.translation, progress),
                                             Math::SLerp (source.rotation, target.rotation, progress),
                                             Math::Lerp (source.scale, target.scale, progress)});

        transform->visualTransformLastSyncTimeNs = time->normalTimeNs;
        transform->visualTransformSyncNeeded =
            transform->visualTransformLastSyncTimeNs < transform->logicalTransformLastObservationTimeNs;
        ++transformCursor;
    }
}

namespace VisualTransformSync
{
const Memory::UniqueString Checkpoint::SYNC_FINISHED {"Transform3dVisualSyncFinished"};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (Memory::UniqueString {"Transform3dVisualSync"})
        .SetExecutor<Transform3dVisualSynchronizer> ();
}
} // namespace VisualTransformSync
} // namespace Emergence::Celerity
