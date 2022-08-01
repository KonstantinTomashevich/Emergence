#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::HierarchyCleanup
{
/// \brief Contains checkpoints, supported by tasks from ::AddToFixedUpdate and ::AddToNormalUpdate.
/// \warning By default, there is no dependency between detachment detection and detached transform removal.
///          User is expected to decide what is executed first depending on how user pipeline graph is organized.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Detached transform detection is started after this checkpoint.
    static const Emergence::Memory::UniqueString DETACHMENT_DETECTION_STARTED;

    /// \brief Detached transform detection is guaranteed to be finished after this checkpoint.
    static const Emergence::Memory::UniqueString DETACHMENT_DETECTION_FINISHED;

    /// \brief Detached transform removal is executed after this checkpoint.
    static const Emergence::Memory::UniqueString DETACHED_REMOVAL_STARTED;

    /// \brief Detached transform removal is guaranteed to be finished after this checkpoint.
    static const Emergence::Memory::UniqueString DETACHED_REMOVAL_FINISHED;
};

/// \brief Adds tasks required to cleanup detached transform hierarchy to fixed update pipeline.
void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept;

/// \brief Adds tasks required to cleanup detached transform hierarchy to normal update pipeline.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::HierarchyCleanup
