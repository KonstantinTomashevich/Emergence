#pragma once

#include <CelerityAssetLogicApi.hpp>

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::AssetManagement
{
/// \brief Contains checkpoints, supported by tasks from ::AddToNormalUpdate.
struct CelerityAssetLogicApi Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Asset management tasks are started after this checkpoint.
    static const Memory::UniqueString STARTED;

    /// \brief Asset type loaders should depend on this checkpoint.
    static const Memory::UniqueString ASSET_LOADING_STARTED;

    /// \brief Asset type loaders should be dependencies of this checkpoint.
    static const Memory::UniqueString ASSET_LOADING_FINISHED;

    /// \brief Asset management tasks are finished before this checkpoint.
    static const Memory::UniqueString FINISHED;
};

/// \brief Adds asset management checkpoints and tasks to normal update pipeline.
CelerityAssetLogicApi void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                                              const AssetReferenceBindingList &_bindingList,
                                              const AssetReferenceBindingEventMap &_bindingEvents) noexcept;
} // namespace Emergence::Celerity::AssetManagement
