#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <Resource/Object/TypeManifest.hpp>

#include <ResourceProvider/ResourceProvider.hpp>

namespace Emergence::Celerity::ResourceObjectLoading
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

/// \brief Adds tasks that execute loading pipeline: process requests and integrate
///        Resource::Object::LibraryLoader with Celerity::Assembly.
void AddToLoadingPipeline (PipelineBuilder &_builder,
                           ResourceProvider::ResourceProvider *_resourceProvider,
                           Resource::Object::TypeManifest _typeManifest) noexcept;
} // namespace Emergence::Celerity::ResourceObjectLoading
