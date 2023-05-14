#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Localization
{
/// \brief Contains checkpoints, supported by tasks from ::AddToNormalUpdate.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief After this checkpoint localization synchronization is started.
    static const Memory::UniqueString SYNC_STARTED;

    /// \brief Localization synchronization is finished before this checkpoint.
    static const Memory::UniqueString SYNC_FINISHED;
};

/// \brief Adds tasks for localization management and synchronization.
///
/// \param _localizationRootPath Known root folder for locale-specific folders. Files for every locale must be
///                              stored in `{_localizationRootPath}/{localeName}/*`.
void AddToNormalUpdate (PipelineBuilder &_builder,
                        Memory::UniqueString _localizationRootPath) noexcept;
} // namespace Emergence::Celerity::Localization
