#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Localization
{
/// \brief Names of files used for storing localization data.
struct Files final
{
    Files () = delete;

    /// \brief Name of file that stores localized strings (without extension).
    /// \details Binary representation (.bin extension) stores keys and values as plain strings.
    ///          Yaml representation (.yaml extension) uses string mapping format.
    static const Memory::UniqueString STRINGS;
};

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
/// \param _maxLoadingTimePerFrameNs Maximum time per frame allocated for locale loading.
void AddToNormalUpdate (PipelineBuilder &_builder,
                        Memory::UniqueString _localizationRootPath,
                        uint64_t _maxLoadingTimePerFrameNs) noexcept;
} // namespace Emergence::Celerity::Localization
