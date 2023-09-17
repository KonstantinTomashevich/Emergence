#pragma once

#include <CelerityLocaleModelApi.hpp>

#include <Celerity/Locale/LocaleConfiguration.hpp>
#include <Celerity/Standard/ContextEscape.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief States of locale loading for LocaleLoadingSharedState synchronization.
enum class LocaleLoadingState : std::uint8_t
{
    /// \brief Loading routine is executing.
    LOADING,

    /// \brief Loading finished successfully.
    SUCCESSFUL,

    /// \brief Loading finished with errors.
    FAILED,
};

/// \brief Contains locale loading state that is shared with background job.
class CelerityLocaleModelApi LocaleLoadingSharedState final : public ContextEscape<LocaleLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "LocaleLoading";

    /// \brief Describes in which state locale loading is right now.
    std::atomic<LocaleLoadingState> loadingState = LocaleLoadingState::LOADING;

    /// \brief Configuration that is currently being loaded from resources.
    LocaleConfiguration configurationInLoading;
};

/// \brief Stores global configuration and state of localization routine.
struct CelerityLocaleModelApi LocaleSingleton final
{
    /// \brief Target locale to be loaded and applied. Edit this field if you need to change locale.
    Memory::UniqueString targetLocale;

    /// \brief Locale that is currently loaded. Designed to be readonly for users.
    Memory::UniqueString loadedLocale;

    /// \brief Locale that is currently being loaded if any. Designed to be readonly for users.
    Memory::UniqueString loadingLocale;

    /// \brief Shared state for background loading through job dispatcher.
    Handling::Handle<LocaleLoadingSharedState> sharedState {new LocaleLoadingSharedState};

    struct CelerityLocaleModelApi Reflection final
    {
        StandardLayout::FieldId targetLocale;
        StandardLayout::FieldId loadedLocale;
        StandardLayout::FieldId loadingLocale;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
