#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace LevelLoading
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString STARTED;
    static const Emergence::Memory::UniqueString FINISHED;
};

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace LevelLoading
