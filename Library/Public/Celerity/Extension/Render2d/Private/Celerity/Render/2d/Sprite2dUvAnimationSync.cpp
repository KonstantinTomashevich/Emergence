#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Batching2d.hpp>
#include <Celerity/Render/2d/Batching2dSingleton.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimation.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimationComponent.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimationSync.hpp>
#include <Celerity/Render/2d/WorldRendering2d.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::Sprite2dUvAnimationSync
{
class Synchronizer final : public TaskExecutorBase<Synchronizer>
{
public:
    Synchronizer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    FetchSingletonQuery fetchBatching;
    FetchSingletonQuery fetchTime;

    ModifyValueQuery modifyAnimationBySpriteId;
    FetchValueQuery fetchAnimationAssetById;
    EditValueQuery editSpriteBySpriteId;
};

Synchronizer::Synchronizer (TaskConstructor &_constructor) noexcept
    : fetchBatching (FETCH_SINGLETON (Batching2dSingleton)),
      fetchTime (FETCH_SINGLETON (TimeSingleton)),

      modifyAnimationBySpriteId (MODIFY_VALUE_1F (Sprite2dUvAnimationComponent, spriteId)),
      fetchAnimationAssetById (FETCH_VALUE_1F (Sprite2dUvAnimation, assetId)),
      editSpriteBySpriteId (_constructor.EditValuePartial (
          Sprite2dComponent::Reflect ().mapping,
          {Sprite2dComponent::Reflect ().spriteId},
          {Sprite2dComponent::Reflect ().materialInstanceId, Sprite2dComponent::Reflect ().uv}))
{
    _constructor.DependOn (Batching2d::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (WorldRendering2d::Checkpoint::STARTED);
}

void Synchronizer::Execute () noexcept
{
    auto batchingCursor = fetchBatching.Execute ();
    const auto *batching = static_cast<const Batching2dSingleton *> (*batchingCursor);

    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const TimeSingleton *> (*timeCursor);

    for (const ViewportInfoContainer &viewport : batching->viewports)
    {
        for (const Batch2d &batch : viewport.batches)
        {
            for (UniqueId spriteId : batch.sprites)
            {
                auto animationCursor = modifyAnimationBySpriteId.Execute (&spriteId);
                auto *animation = static_cast<Sprite2dUvAnimationComponent *> (*animationCursor);

                if (!animation || animation->lastSyncNormalTimeNs == time->realNormalTimeNs)
                {
                    continue;
                }

                animation->lastSyncNormalTimeNs = time->realNormalTimeNs;
                if (animation->tickTime)
                {
                    animation->currentTimeNs += static_cast<uint64_t> (time->normalDurationS * 1e9f);
                }

                auto animationAssetCursor = fetchAnimationAssetById.Execute (&animation->animationId);
                const auto *animationAsset = static_cast<const Sprite2dUvAnimation *> (*animationAssetCursor);

                if (!animationAsset || animationAsset->frames.empty ())
                {
                    continue;
                }

                const Sprite2dUvAnimationFrame &lastFrame = animationAsset->frames.back ();
                const uint64_t animationDuration = lastFrame.startTimeNs + lastFrame.durationNs;
                animation->finished = animation->currentTimeNs >= animationDuration;

                if (animation->finished && animation->loop)
                {
                    animation->currentTimeNs %= animationDuration;
                    animation->finished = false;
                }

                // Time might be changed from outside sources, so we cannot just increment/decrement frame counter.
                // Therefore, we're doing binary search to get current frame. It is still quite fast.

                auto nextFrameIterator = std::upper_bound (
                    animationAsset->frames.begin (), animationAsset->frames.end (), animation->currentTimeNs,
                    [] (uint64_t _currentTimeNs, const Sprite2dUvAnimationFrame &_frame)
                    {
                        return _currentTimeNs < _frame.startTimeNs;
                    });

                auto currentFrameIterator =
                    nextFrameIterator == animationAsset->frames.begin () ? nextFrameIterator : nextFrameIterator - 1u;
                const auto currentFrameIndex =
                    static_cast<uint32_t> (currentFrameIterator - animationAsset->frames.begin ());

                if (currentFrameIndex != animation->currentFrame)
                {
                    animation->currentFrame = currentFrameIndex;
                    auto spriteCursor = editSpriteBySpriteId.Execute (&spriteId);

                    if (auto *sprite = static_cast<Sprite2dComponent *> (*spriteCursor))
                    {
                        sprite->materialInstanceId = animationAsset->materialInstanceId;
                        sprite->uv = animationAsset->frames[currentFrameIndex].uv;
                    }
                    else
                    {
                        // Delete animation that is no longer attached to sprite.
                        ~animationCursor;
                    }
                }
            }
        }
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Memory::Literals;

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Sprite2dUvAnimationSync");
    _pipelineBuilder.AddTask ("Sprite2dUvAnimationSynchronizer"_us).SetExecutor<Synchronizer> ();
}
} // namespace Emergence::Celerity::Sprite2dUvAnimationSync
