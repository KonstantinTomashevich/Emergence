#pragma once

#include <Celerity/Locale/LocaleConfiguration.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Stores global configuration and state of localization routine.
struct LocaleSingleton final
{
    /// \brief Target locale to be loaded and applied. Edit this field if you need to change locale.
    Memory::UniqueString targetLocale;

    /// \brief Locale that is currently loaded. Designed to be readonly for users.
    Memory::UniqueString loadedLocale;

    /// \brief Locale that is currently being loaded if any. Designed to be readonly for users.
    Memory::UniqueString loadingLocale;

    /// \brief Configuration that is currently being loaded from resources.
    /// \warning Used from loading routine, therefore unsafe to access unless ::isConfigurationLoaded.
    LocaleConfiguration configurationInLoading;

    /// \brief Whether ::configurationInLoading is already loaded.
    std::atomic_flag isConfigurationLoaded;

    struct Reflection final
    {
        StandardLayout::FieldId targetLocale;
        StandardLayout::FieldId loadedLocale;
        StandardLayout::FieldId loadingLocale;
        StandardLayout::FieldId configurationInLoading;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
