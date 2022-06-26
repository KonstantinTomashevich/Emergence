#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
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
    EditSignalQuery editTransformsWithUpdateFlag;
};

Transform3dVisualSynchronizer::Transform3dVisualSynchronizer (TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (TimeSingleton)),
      editTransformsWithUpdateFlag (EDIT_SIGNAL (Transform3dComponent, visualTransformSyncNeeded, true))
{
    _constructor.MakeDependencyOf (VisualTransformSync::Checkpoint::SYNC_FINISHED);
}

void Transform3dVisualSynchronizer::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const TimeSingleton *> (*timeCursor);
    auto transformCursor = editTransformsWithUpdateFlag.Execute ();

    while (auto *transform = static_cast<Transform3dComponent *> (*transformCursor))
    {
        if (transform->lastObservedLogicalTransformRevision != transform->logicalLocalTransformRevision)
        {
            transform->lastObservedLogicalTransformRevision = transform->logicalLocalTransformRevision;
            transform->logicalTransformLastObservationTimeNs = time->fixedTimeNs;
        }

        const Math::Transform3d &source = transform->GetVisualLocalTransform ();
        const Math::Transform3d &target = transform->GetLogicalLocalTransform ();

        if (Math::NearlyEqual (source, target))
        {
            // Interpolation skip detected: mark that we already see object in its final transform.
            transform->visualTransformLastSyncTimeNs = transform->logicalTransformLastObservationTimeNs;
        }
        else
        {
            // If we never interpolated this object before, treat it as synced during last frame.
            if (transform->visualTransformLastSyncTimeNs == 0u)
            {
                transform->visualTransformLastSyncTimeNs =
                    time->normalTimeNs - static_cast<uint64_t> (time->normalDurationS * 1000000000.0f);
            }

            // Apply interpolation.
            const uint64_t elapsed = time->normalTimeNs - transform->visualTransformLastSyncTimeNs;

            const uint64_t duration =
                transform->logicalTransformLastObservationTimeNs - transform->visualTransformLastSyncTimeNs;

            const float progress =
                Math::Clamp (static_cast<float> (elapsed) / static_cast<float> (duration), 0.0f, 1.0f);

            transform->SetVisualLocalTransform ({Math::Lerp (source.translation, target.translation, progress),
                                                 Math::SLerp (source.rotation, target.rotation, progress),
                                                 Math::Lerp (source.scale, target.scale, progress)});

            transform->visualTransformLastSyncTimeNs = time->normalTimeNs;
        }

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
