#pragma once

#include <Celerity/Asset/Config/TypeMeta.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::AssetConfigLoading
{
/// \brief Contains checkpoints, supported by tasks from ::AddToLoadingPipeline.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Loading pipeline processing starts after this checkpoint.
    static const Memory::UniqueString STARTED;

    /// \brief Loading pipeline processing is finished before this checkpoint.
    static const Memory::UniqueString FINISHED;
};

/// \brief Adds tasks that execute loading pipeline: process requests and load configs.
void AddToLoadingPipeline (PipelineBuilder &_builder,
                           std::uint64_t _maxLoadingTimePerFrameNs,
                           const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept;
} // namespace Emergence::Celerity::AssetConfigLoading
