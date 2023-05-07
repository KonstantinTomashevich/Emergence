#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Batching2d.hpp>
#include <Celerity/Render/2d/Batching2dSingleton.hpp>
#include <Celerity/Render/2d/Events.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimation.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimationComponent.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimationSync.hpp>
#include <Celerity/Render/2d/WorldRendering2d.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/2d/BoundsCalculation2d.hpp>

namespace Emergence::Celerity::Sprite2dUvAnimationSync
{
class PreBatchingSynchronizer : public TaskExecutorBase<PreBatchingSynchronizer>
{
public:
    PreBatchingSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    enum class SyncResult
    {
        SPRITE_INVALID,
        POSTPONED,
        DONE,
    };

    SyncResult DoSync (Sprite2dUvAnimationComponent *_animation) noexcept;

    FetchSequenceQuery fetchAnimationAddedEvents;
    FetchSequenceQuery fetchAnimationChangedEvents;
    ModifySignalQuery modifyWaitingForAnimationToLoad;

    ModifyValueQuery modifyAnimationBySpriteId;
    FetchValueQuery fetchAnimationAssetById;
    EditValueQuery editSpriteBySpriteId;
};

PreBatchingSynchronizer::PreBatchingSynchronizer (TaskConstructor &_constructor) noexcept
    : fetchAnimationAddedEvents (FETCH_SEQUENCE (Sprite2dUvAnimationAddedNormalEvent)),
      fetchAnimationChangedEvents (FETCH_SEQUENCE (Sprite2dUvAnimationSyncedValuesChangedNormalEvent)),
      modifyWaitingForAnimationToLoad (
          _constructor.ModifySignalPartial (Sprite2dUvAnimationComponent::Reflect ().mapping,
                                            Sprite2dUvAnimationComponent::Reflect ().waitingForAnimationToLoad,
                                            array_cast<bool, sizeof (uint64_t)> (true),
                                            {
                                                Sprite2dUvAnimationComponent::Reflect ().currentFrame,
                                                Sprite2dUvAnimationComponent::Reflect ().waitingForAnimationToLoad,
                                            })),

      modifyAnimationBySpriteId (
          _constructor.ModifyValuePartial (Sprite2dUvAnimationComponent::Reflect ().mapping,
                                           {Sprite2dUvAnimationComponent::Reflect ().spriteId},
                                           {
                                               Sprite2dUvAnimationComponent::Reflect ().currentFrame,
                                               Sprite2dUvAnimationComponent::Reflect ().waitingForAnimationToLoad,
                                           })),
      fetchAnimationAssetById (FETCH_VALUE_1F (Sprite2dUvAnimation, assetId)),
      editSpriteBySpriteId (_constructor.EditValuePartial (Sprite2dComponent::Reflect ().mapping,
                                                           {Sprite2dComponent::Reflect ().spriteId},
                                                           {Sprite2dComponent::Reflect ().materialInstanceId}))
{
    _constructor.DependOn (RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
    _constructor.MakeDependencyOf (BoundsCalculation2d::Checkpoint::STARTED);
}

void PreBatchingSynchronizer::Execute () noexcept
{
    for (auto eventCursor = fetchAnimationAddedEvents.Execute ();
         const auto *event = static_cast<const Sprite2dUvAnimationAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        auto animationCursor = modifyAnimationBySpriteId.Execute (&event->spriteId);
        if (auto *animation = static_cast<Sprite2dUvAnimationComponent *> (*animationCursor))
        {
            if (DoSync (animation) == SyncResult::SPRITE_INVALID)
            {
                ~animationCursor;
            }
        }
    }

    for (auto eventCursor = fetchAnimationChangedEvents.Execute ();
         const auto *event = static_cast<const Sprite2dUvAnimationSyncedValuesChangedNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto animationCursor = modifyAnimationBySpriteId.Execute (&event->spriteId);
        if (auto *animation = static_cast<Sprite2dUvAnimationComponent *> (*animationCursor))
        {
            if (DoSync (animation) == SyncResult::SPRITE_INVALID)
            {
                ~animationCursor;
            }
        }
    }

    for (auto animationCursor = modifyWaitingForAnimationToLoad.Execute ();
         auto *animation = static_cast<Sprite2dUvAnimationComponent *> (*animationCursor);)
    {
        if (DoSync (animation) == SyncResult::SPRITE_INVALID)
        {
            ~animationCursor;
        }
        else
        {
            ++animationCursor;
        }
    }
}

PreBatchingSynchronizer::SyncResult PreBatchingSynchronizer::DoSync (Sprite2dUvAnimationComponent *_animation) noexcept
{
    auto animationAssetCursor = fetchAnimationAssetById.Execute (&_animation->animationId);
    const auto *animationAsset = static_cast<const Sprite2dUvAnimation *> (*animationAssetCursor);

    if (!animationAsset || animationAsset->frames.empty ())
    {
        _animation->waitingForAnimationToLoad = true;
        return SyncResult::POSTPONED;
    }

    _animation->waitingForAnimationToLoad = false;
    auto spriteCursor = editSpriteBySpriteId.Execute (&_animation->spriteId);
    auto *sprite = static_cast<Sprite2dComponent *> (*spriteCursor);

    if (!sprite)
    {
        return SyncResult::SPRITE_INVALID;
    }

    sprite->materialInstanceId = animationAsset->materialInstanceId;
    // Reset frame to force animation sync on post batch phase.
    _animation->currentFrame = std::numeric_limits<decltype (_animation->currentFrame)>::max ();
    return PreBatchingSynchronizer::SyncResult::DONE;
}

class PostBatchingSynchronizer final : public TaskExecutorBase<PostBatchingSynchronizer>
{
public:
    PostBatchingSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    FetchSingletonQuery fetchBatching;
    FetchSingletonQuery fetchTime;

    ModifyValueQuery modifyAnimationBySpriteId;
    FetchValueQuery fetchAnimationAssetById;
    EditValueQuery editSpriteBySpriteId;
};

PostBatchingSynchronizer::PostBatchingSynchronizer (TaskConstructor &_constructor) noexcept
    : fetchBatching (FETCH_SINGLETON (Batching2dSingleton)),
      fetchTime (FETCH_SINGLETON (TimeSingleton)),

      modifyAnimationBySpriteId (
          _constructor.ModifyValuePartial (Sprite2dUvAnimationComponent::Reflect ().mapping,
                                           {Sprite2dUvAnimationComponent::Reflect ().spriteId},
                                           {
                                               Sprite2dUvAnimationComponent::Reflect ().lastSyncNormalTimeNs,
                                               Sprite2dUvAnimationComponent::Reflect ().currentTimeNs,
                                               Sprite2dUvAnimationComponent::Reflect ().currentFrame,
                                               Sprite2dUvAnimationComponent::Reflect ().finished,
                                           })),
      fetchAnimationAssetById (FETCH_VALUE_1F (Sprite2dUvAnimation, assetId)),
      editSpriteBySpriteId (_constructor.EditValuePartial (Sprite2dComponent::Reflect ().mapping,
                                                           {Sprite2dComponent::Reflect ().spriteId},
                                                           {Sprite2dComponent::Reflect ().uv}))
{
    _constructor.DependOn (Batching2d::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (WorldRendering2d::Checkpoint::STARTED);
}

void PostBatchingSynchronizer::Execute () noexcept
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
                        EMERGENCE_ASSERT (sprite->materialInstanceId == animationAsset->materialInstanceId);
                        const Math::AxisAlignedBox2d &uv = animationAsset->frames[currentFrameIndex].uv;

                        if (animation->flipU)
                        {
                            sprite->uv.min.x = uv.max.x;
                            sprite->uv.max.x = uv.min.x;
                        }
                        else
                        {
                            sprite->uv.min.x = uv.min.x;
                            sprite->uv.max.x = uv.max.x;
                        }

                        if (animation->flipV)
                        {
                            sprite->uv.min.y = uv.max.y;
                            sprite->uv.max.y = uv.min.y;
                        }
                        else
                        {
                            sprite->uv.min.y = uv.min.y;
                            sprite->uv.max.y = uv.max.y;
                        }
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
    _pipelineBuilder.AddTask ("Sprite2dUvAnimationPreBatchingSynchronizer"_us).SetExecutor<PreBatchingSynchronizer> ();
    _pipelineBuilder.AddTask ("Sprite2dUvAnimationPostBatchingSynchronizer"_us)
        .SetExecutor<PostBatchingSynchronizer> ();
}
} // namespace Emergence::Celerity::Sprite2dUvAnimationSync
