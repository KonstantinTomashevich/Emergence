#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::VisualTransformSync
{
/// \brief Contains checkpoints, supported by tasks from ::AddToNormalUpdate.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief After this checkpoint visual transforms are guaranteed to be ready for use.
    static const Emergence::Memory::UniqueString FINISHED;
};

/// \brief Adds tasks required to sync and interpolate visual transforms from logical transforms.
/// \see Transform3dComponent
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::VisualTransformSync
