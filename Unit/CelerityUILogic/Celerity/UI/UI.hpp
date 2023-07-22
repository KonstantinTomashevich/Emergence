#pragma once

#include <CelerityUILogicApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

#include <InputStorage/FrameInputAccumulator.hpp>

namespace Emergence::Celerity::UI
{
/// \brief Contains checkpoints, supported by tasks from ::AddToNormalUpdate.
struct CelerityUILogicApi Checkpoint final
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
CelerityUILogicApi void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                                           InputStorage::FrameInputAccumulator *_inputAccumulator,
                                           const InputStorage::KeyCodeMapping &_keyCodeMapping) noexcept;
} // namespace Emergence::Celerity::UI
