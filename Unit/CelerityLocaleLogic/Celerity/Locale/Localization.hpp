#pragma once

#include <CelerityLocaleLogicApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

namespace Emergence::Celerity::Localization
{
/// \brief Contains checkpoints, supported by tasks from ::AddToNormalUpdate.
struct CelerityLocaleLogicApi Checkpoint final
{
    Checkpoint () = delete;

    /// \brief After this checkpoint localization synchronization is started.
    static const Memory::UniqueString SYNC_STARTED;

    /// \brief Localization synchronization is finished before this checkpoint.
    static const Memory::UniqueString SYNC_FINISHED;
};

/// \brief Adds tasks for localization management and synchronization.
CelerityLocaleLogicApi void AddToNormalUpdate (PipelineBuilder &_builder,
                                               Resource::Provider::ResourceProvider *_resourceProvider) noexcept;
} // namespace Emergence::Celerity::Localization
