#pragma once

#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Resource/Config/TypeMeta.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

namespace Emergence::Celerity::ResourceConfigLoading
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
                           Resource::Provider::ResourceProvider *_resourceProvider,
                           const Container::Vector<ResourceConfigTypeMeta> &_supportedTypes) noexcept;
} // namespace Emergence::Celerity::ResourceConfigLoading
