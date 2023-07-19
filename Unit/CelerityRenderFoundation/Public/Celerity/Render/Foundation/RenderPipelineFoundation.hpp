#pragma once

#include <CelerityRenderFoundationApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::RenderPipelineFoundation
{
/// \brief Contains checkpoints, supported by tasks from ::AddToNormalUpdate.
struct CelerityRenderFoundationApi Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Render pipeline is started after this checkpoint.
    static const Memory::UniqueString RENDER_STARTED;

    /// \brief Viewport object to Render::Backend::Viewport sync is done after this checkpoint.
    static const Memory::UniqueString VIEWPORT_SYNC_STARTED;

    /// \brief Viewport object to Render::Backend::Viewport sync is finished before this checkpoint.
    static const Memory::UniqueString VIEWPORT_SYNC_FINISHED;

    /// \brief All render pipeline tasks must be finished before this checkpoint.
    /// \details After this checkpoint all render commands are submitted,
    ///          therefore RenderFoundationSingleton is no longer available.
    static const Memory::UniqueString RENDER_FINISHED;
};

/// \brief Adds tasks that update rendering data and perform rendering.
CelerityRenderFoundationApi void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::RenderPipelineFoundation
