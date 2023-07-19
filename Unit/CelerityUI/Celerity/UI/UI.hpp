#pragma once

#include <CelerityUIApi.hpp>

#include <Celerity/Input/FrameInputAccumulator.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::UI
{
/// \brief Contains checkpoints, supported by tasks from ::AddToNormalUpdate.
struct CelerityUIApi Checkpoint final
{
    Checkpoint () = delete;

    /// \brief UI nodes hierarchy cleanup on removal is started after this checkpoint.
    static const Memory::UniqueString HIERARCHY_CLEANUP_STARTED;

    /// \brief UI nodes hierarchy cleanup on removal is finished before this checkpoint.
    static const Memory::UniqueString HIERARCHY_CLEANUP_FINISHED;

    /// \brief After this checkpoint input processing and UI structure update is executed.
    static const Memory::UniqueString UPDATE_STARTED;

    /// \brief Before this checkpoint execution of UI structure update and input processing is done.
    /// \details Happens before input dispatch is done.
    static const Memory::UniqueString UPDATE_FINISHED;

    /// \brief UI rendering is executed after this checkpoint.
    static const Memory::UniqueString RENDER_STARTED;

    /// \brief UI rendering is executed before this checkpoint.
    /// \details Happens inside render foundation pipeline.
    static const Memory::UniqueString RENDER_FINISHED;
};

/// \brief Adds tasks that update and render user interface.
CelerityUIApi void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                                      FrameInputAccumulator *_inputAccumulator,
                                      const KeyCodeMapping &_keyCodeMapping) noexcept;
} // namespace Emergence::Celerity::UI
