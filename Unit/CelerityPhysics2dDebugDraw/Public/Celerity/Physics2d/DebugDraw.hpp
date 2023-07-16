#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Physics2dDebugDraw
{
/// \brief Contains checkpoints, supported by tasks from ::AddToFixedUpdate.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Debug draw shape management starts after this checkpoint.
    static const Emergence::Memory::UniqueString STARTED;

    /// \brief Debug draw shape management finishes before this checkpoint.
    static const Emergence::Memory::UniqueString FINISHED;
};

/// \brief Adds tasks that manage debug draw shapes for physics collision shapes.
void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::Physics2dDebugDraw
