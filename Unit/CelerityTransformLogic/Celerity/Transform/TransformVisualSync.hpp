#pragma once

#include <CelerityTransformLogicApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::TransformVisualSync
{
/// \brief Contains checkpoints, supported by tasks from ::Add2dToNormalUpdate and ::Add3dToNormalUpdate.
struct CelerityTransformLogicApi Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Before this checkpoint visual transforms are not ready for use, but new transforms may be added safely.
    static const Memory::UniqueString STARTED;

    /// \brief After this checkpoint visual transforms are guaranteed to be ready for use.
    static const Memory::UniqueString FINISHED;
};

/// \brief Adds tasks required to sync and interpolate visual 2d transforms from logical 2d transforms.
/// \see TransformComponent
CelerityTransformLogicApi void Add2dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;

/// \brief Adds tasks required to sync and interpolate visual 3d transforms from logical 3d transforms.
/// \see TransformComponent
CelerityTransformLogicApi void Add3dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::TransformVisualSync
