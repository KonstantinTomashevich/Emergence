#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Assembly
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString ASSEMBLY_STARTED;

    static const Emergence::Memory::UniqueString ASSEMBLY_FINISHED;
};

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder, const AssemblerConfiguration &_configuration) noexcept;

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const AssemblerConfiguration &_configuration) noexcept;
} // namespace Emergence::Celerity::Assembly
