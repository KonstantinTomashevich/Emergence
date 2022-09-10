#pragma once

#include <Asset/Object/TypeManifest.hpp>

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::AssetObjectLoading
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
///        Asset::Object::LibraryLoader with Celerity::Assembly.
void AddToLoadingPipeline (PipelineBuilder &_builder, Asset::Object::TypeManifest _typeManifest) noexcept;
} // namespace Emergence::Celerity::AssetObjectLoading
