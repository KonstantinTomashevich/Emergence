#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <Math/AxisAlignedBox2d.hpp>

namespace Emergence::Celerity::Rendering2d
{
/// \brief Contains checkpoints, supported by tasks from ::AddToNormalUpdate.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Rendering routine is started after this checkpoint.
    static const Memory::UniqueString STARTED;

    /// \brief Rendering routine is finished after this checkpoint.
    static const Memory::UniqueString FINISHED;
};

/// \brief Adds tasks that update rendering data and perform rendering.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept;
} // namespace Emergence::Celerity::Rendering2d
