#pragma once

#include <Celerity/Assembly/AssemblerConfiguration.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Assembly
{
/// \brief Contains checkpoints, supported by tasks from ::AddToFixedUpdate and ::AddToNormalUpdate.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Assembly routine execution starts after this checkpoint.
    static const Memory::UniqueString STARTED;

    /// \brief Assembly routine execution finishes before this checkpoint.
    static const Memory::UniqueString FINISHED;
};

/// \brief Adds tasks that execute assembly routine in fixed update.
/// \details Implementations for fixed and normal update fire different events and must assemble different types.
/// \invariant Both update routines have equal `_allCustomKeys` vectors.
void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder,
                       const CustomKeyVector &_allCustomKeys,
                       const TypeBindingVector &_fixedUpdateTypes,
                       std::uint64_t _maxAssemblyTimePerFrameNs) noexcept;

/// \brief Adds tasks that execute assembly routine in normal update.
/// \details Implementations for fixed and normal update fire different events and must assemble different types.
/// \invariant Both update routines have equal `_allCustomKeys` vectors.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const CustomKeyVector &_allCustomKeys,
                        const TypeBindingVector &_normalUpdateTypes,
                        std::uint64_t _maxAssemblyTimePerFrameNs) noexcept;
} // namespace Emergence::Celerity::Assembly
