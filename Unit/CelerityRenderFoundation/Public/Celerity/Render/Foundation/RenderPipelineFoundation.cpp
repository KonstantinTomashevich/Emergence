#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Render/Foundation/FrameBuffer.hpp>
#include <Celerity/Render/Foundation/RenderFoundationSingleton.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>

namespace Emergence::Celerity::RenderPipelineFoundation
{
const Memory::UniqueString Checkpoint::RENDER_STARTED {"RenderFoundationRenderStarted"};
const Memory::UniqueString Checkpoint::VIEWPORT_SYNC_STARTED {"RenderFoundationViewportSyncStarted"};
const Memory::UniqueString Checkpoint::VIEWPORT_SYNC_FINISHED {"RenderFoundationViewportSyncFinished"};
const Memory::UniqueString Checkpoint::RENDER_FINISHED {"RenderFoundationRenderFinished"};

class ViewportSynchronizer final : public TaskExecutorBase<ViewportSynchronizer>
{
public:
    ViewportSynchronizer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    FetchSingletonQuery fetchRenderFoundation;
    FetchValueQuery fetchViewportByName;
    FetchValueQuery fetchFrameBufferById;

    FetchSequenceQuery fetchViewportAddedNormalEvent;
    FetchSequenceQuery fetchViewportAddedCustomEvent;
    FetchSequenceQuery fetchViewportChangedEvent;
};

ViewportSynchronizer::ViewportSynchronizer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      fetchRenderFoundation (FETCH_SINGLETON (RenderFoundationSingleton)),
      fetchViewportByName (FETCH_VALUE_1F (Viewport, name)),
      fetchFrameBufferById (FETCH_VALUE_1F (FrameBuffer, assetId)),

      fetchViewportAddedNormalEvent (FETCH_SEQUENCE (ViewportAddedNormalEvent)),
      fetchViewportAddedCustomEvent (FETCH_SEQUENCE (ViewportAddedCustomToNormalEvent)),
      fetchViewportChangedEvent (FETCH_SEQUENCE (ViewportChangedNormalEvent))
{
    _constructor.DependOn (Checkpoint::RENDER_STARTED);
    _constructor.DependOn (Checkpoint::VIEWPORT_SYNC_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::VIEWPORT_SYNC_FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::RENDER_FINISHED);
}

void ViewportSynchronizer::Execute () noexcept
{
    auto renderFoundationCursor = fetchRenderFoundation.Execute ();
    const auto *renderFoundation = static_cast<const RenderFoundationSingleton *> (*renderFoundationCursor);
    const Render::Backend::FrameBuffer invalidFrameBuffer = Render::Backend::FrameBuffer::CreateInvalid ();

    auto fetchFrameBuffer =
        [this, &invalidFrameBuffer] (Memory::UniqueString _frameBufferId) -> const Render::Backend::FrameBuffer &
    {
        if (auto cursor = fetchFrameBufferById.Execute (&_frameBufferId);
            const auto *frameBuffer = static_cast<const FrameBuffer *> (*cursor))
        {
            return frameBuffer->frameBuffer;
        }

        return invalidFrameBuffer;
    };

    for (auto eventCursor = fetchViewportAddedNormalEvent.Execute ();
         const auto *event = static_cast<const ViewportAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        auto cursor = fetchViewportByName.Execute (&event->name);
        if (const auto *viewport = static_cast<const Viewport *> (*cursor))
        {
            viewport->viewport = Render::Backend::Viewport {renderFoundation->renderer};
            viewport->viewport.SubmitConfiguration (fetchFrameBuffer (viewport->targetFrameBuffer), viewport->x,
                                                    viewport->y, viewport->width, viewport->height, viewport->sortMode,
                                                    viewport->clearColor);
        }
    }

    for (auto eventCursor = fetchViewportAddedCustomEvent.Execute ();
         const auto *event = static_cast<const ViewportAddedCustomToNormalEvent *> (*eventCursor); ++eventCursor)
    {
        auto cursor = fetchViewportByName.Execute (&event->name);
        if (const auto *viewport = static_cast<const Viewport *> (*cursor))
        {
            viewport->viewport = Render::Backend::Viewport {renderFoundation->renderer};
            viewport->viewport.SubmitConfiguration (fetchFrameBuffer (viewport->targetFrameBuffer), viewport->x,
                                                    viewport->y, viewport->width, viewport->height, viewport->sortMode,
                                                    viewport->clearColor);
        }
    }

    for (auto eventCursor = fetchViewportChangedEvent.Execute ();
         const auto *event = static_cast<const ViewportChangedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        auto cursor = fetchViewportByName.Execute (&event->name);
        if (const auto *viewport = static_cast<const Viewport *> (*cursor))
        {
            viewport->viewport.SubmitConfiguration (fetchFrameBuffer (viewport->targetFrameBuffer), viewport->x,
                                                    viewport->y, viewport->width, viewport->height, viewport->sortMode,
                                                    viewport->clearColor);
        }
    }
}

class RenderFinalizer final : public TaskExecutorBase<RenderFinalizer>
{
public:
    RenderFinalizer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    ModifySingletonQuery modifyRenderFoundation;
    FetchAscendingRangeQuery fetchViewportBySortIndexAscending;
    Container::Vector<Render::Backend::ViewportId> viewportOrder {Memory::Profiler::AllocationGroup::Top ()};
};

RenderFinalizer::RenderFinalizer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyRenderFoundation (MODIFY_SINGLETON (RenderFoundationSingleton)),
      fetchViewportBySortIndexAscending (FETCH_ASCENDING_RANGE (Viewport, sortIndex))
{
    _constructor.DependOn (Checkpoint::RENDER_FINISHED);
}

void RenderFinalizer::Execute () noexcept
{
    auto renderFoundationCursor = modifyRenderFoundation.Execute ();
    auto *renderFoundation = static_cast<RenderFoundationSingleton *> (*renderFoundationCursor);

    for (auto viewportCursor = fetchViewportBySortIndexAscending.Execute (nullptr, nullptr);
         const auto *viewport = static_cast<const Viewport *> (*viewportCursor); ++viewportCursor)
    {
        viewportOrder.emplace_back (viewport->viewport.GetId ());
    }

    if (!viewportOrder.empty ())
    {
        renderFoundation->renderer.SubmitViewportOrder (viewportOrder);
        viewportOrder.clear ();
    }

    renderFoundation->renderer.SubmitFrame ();
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Memory::Literals;

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("RenderPipelineFoundation");
    _pipelineBuilder.AddCheckpoint (Checkpoint::RENDER_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::VIEWPORT_SYNC_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::VIEWPORT_SYNC_FINISHED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::RENDER_FINISHED);

    _pipelineBuilder.AddTask ("RenderFoundationViewportSynchronizer"_us).SetExecutor<ViewportSynchronizer> ();
    _pipelineBuilder.AddTask ("RenderFoundationRenderFinalizer"_us).SetExecutor<RenderFinalizer> ();
}
} // namespace Emergence::Celerity::RenderPipelineFoundation
