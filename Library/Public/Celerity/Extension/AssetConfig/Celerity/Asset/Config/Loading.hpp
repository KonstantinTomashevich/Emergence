#pragma once

#include <Celerity/Asset/Config/TypeMeta.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::AssetConfigLoading
{
/// \brief Contains checkpoints, supported by tasks from ::AddToLoadingPipeline.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Config loading routine starts after this checkpoint.
    static const Memory::UniqueString STARTED;

    /// \brief Config loading routine is finished before this checkpoint.
    static const Memory::UniqueString FINISHED;
};

/// \brief Adds task that executes loading routine: processes requests and loads configs.
void AddToLoadingPipeline (PipelineBuilder &_builder,
                           std::uint64_t _maxLoadingTimePerFrameNs,
                           const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept;
} // namespace Emergence::Celerity::AssetConfigLoading
