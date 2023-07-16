#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::TransformHierarchyCleanup
{
/// \brief Contains checkpoints, supported by tasks from ::Add2dToFixedUpdate, ::Add3dToFixedUpdate,
///        ::Add2dToNormalUpdate. and ::Add3dToNormalUpdate.
/// \details For game world objects it is intuitive to treat transform components as cornerstone: if transform
///          component is removed, then all other components and all child transforms should be recursively
///          cleaned up too. TransformHierarchyCleanup implements this feature for transforms and leaves space
///          for users to insert cleaners for their own components between ::CLEANUP_STARTED and ::FINISHED
///          checkpoints. That provides centralized way to apply transform hierarchy cleanups.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Detached transform detection and removal is started after this checkpoint.
    static const Memory::UniqueString STARTED;

    /// \brief Removal of detached components is started after this checkpoint.
    static const Memory::UniqueString CLEANUP_STARTED;

    /// \brief Detached transform and component detection and removal is finished before this checkpoint.
    static const Memory::UniqueString FINISHED;
};

/// \brief Adds tasks required to cleanup detached transform 2d hierarchy to fixed update pipeline.
void Add2dToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept;

/// \brief Adds tasks required to cleanup detached transform 2d hierarchy to normal update pipeline.
void Add2dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;

/// \brief Adds tasks required to cleanup detached transform 3d hierarchy to fixed update pipeline.
void Add3dToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept;

/// \brief Adds tasks required to cleanup detached transform 3d hierarchy to normal update pipeline.
void Add3dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::TransformHierarchyCleanup
