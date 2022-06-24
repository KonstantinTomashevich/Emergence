#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Math/Scalar.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dVisualSync.hpp>

namespace Emergence::Transform
{
class Transform3dVisualSynchronizer final : public Celerity::TaskExecutorBase<Transform3dVisualSynchronizer>
{
public:
    Transform3dVisualSynchronizer (Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Celerity::FetchSingletonQuery fetchTime;
    Celerity::EditSignalQuery editTransformsWithUpdateFlag;
};

Transform3dVisualSynchronizer::Transform3dVisualSynchronizer (Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Celerity::TimeSingleton)),
      editTransformsWithUpdateFlag (EDIT_SIGNAL (Transform3dComponent, visualTransformSyncNeeded, true))
{
    _constructor.MakeDependencyOf (VisualSync::Checkpoint::SYNC_FINISHED);
}

void Transform3dVisualSynchronizer::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Celerity::TimeSingleton *> (*timeCursor);
    auto transformCursor = editTransformsWithUpdateFlag.Execute ();

    while (auto *transform = static_cast<Transform3dComponent *> (*transformCursor))
    {
        if (transform->lastObservedLogicalTransformRevision != transform->logicalLocalTransformRevision)
        {
            transform->lastObservedLogicalTransformRevision = transform->logicalLocalTransformRevision;
            transform->logicalTransformLastObservationTimeNs = time->fixedTimeNs;
        }

        if (transform->interpolationSkipRequested)
        {
            // Just teleport object.
            transform->SetVisualLocalTransform (transform->GetLogicalLocalTransform ());
            transform->visualTransformLastSyncTimeNs = transform->logicalTransformLastObservationTimeNs;
            transform->interpolationSkipRequested = false;
        }
        else
        {
            // Apply interpolation.
            const uint64_t elapsed = time->normalTimeNs - transform->visualTransformLastSyncTimeNs;

            const uint64_t duration =
                transform->logicalTransformLastObservationTimeNs - transform->visualTransformLastSyncTimeNs;

            const float progress =
                Math::Clamp (static_cast<float> (elapsed) / static_cast<float> (duration), 0.0f, 1.0f);

            const Math::Transform3d &source = transform->GetVisualLocalTransform ();
            const Math::Transform3d &target = transform->GetLogicalLocalTransform ();

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

namespace VisualSync
{
const Memory::UniqueString Checkpoint::SYNC_FINISHED {"Transform3dVisualSyncFinished"};

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask (Memory::UniqueString {"Transform3dVisualSync"});
    constructor.SetExecutor<Transform3dVisualSynchronizer> ();
}
} // namespace VisualSync
} // namespace Emergence::Transform
