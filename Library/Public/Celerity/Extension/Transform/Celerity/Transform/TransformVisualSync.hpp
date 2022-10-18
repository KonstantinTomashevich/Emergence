#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::VisualTransformSync
{
/// \brief Contains checkpoints, supported by tasks from ::Add2dToNormalUpdate and ::Add3dToNormalUpdate.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief After this checkpoint visual transforms are guaranteed to be ready for use.
    static const Emergence::Memory::UniqueString FINISHED;
};

/// \brief Adds tasks required to sync and interpolate visual 2d transforms from logical 2d transforms.
/// \see TransformComponent
void Add2dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;

/// \brief Adds tasks required to sync and interpolate visual 3d transforms from logical 3d transforms.
/// \see TransformComponent
void Add3dToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::VisualTransformSync
