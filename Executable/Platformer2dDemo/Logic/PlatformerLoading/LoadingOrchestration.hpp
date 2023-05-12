#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <Framework/GameState.hpp>

namespace PlatformerLoadingOrchestration
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString STARTED;
    static const Emergence::Memory::UniqueString FINISHED;
};

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        const ViewDropHandle &_viewDropHandle,
                        Emergence::Celerity::WorldView *_ownerView) noexcept;
} // namespace PlatformerLoadingOrchestration
