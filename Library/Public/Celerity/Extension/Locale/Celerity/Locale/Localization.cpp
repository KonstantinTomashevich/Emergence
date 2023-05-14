#include <filesystem>
#include <fstream>

#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/Locale/Localization.hpp>
#include <Celerity/Locale/LocalizedString.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

namespace Emergence::Celerity::Localization
{
const Memory::UniqueString Checkpoint::SYNC_STARTED {"Localization::SyncStarted"};
const Memory::UniqueString Checkpoint::SYNC_FINISHED {"Localization::SyncFinished"};

class LocalizationSynchronizer final : public TaskExecutorBase<LocalizationSynchronizer>
{
public:
    LocalizationSynchronizer (TaskConstructor &_constructor, Memory::UniqueString _localizationRootPath) noexcept;

    void Execute () noexcept;

private:
    void SyncLocaleRequest (LocaleSingleton *_locale) noexcept;

    void UpdateLocaleLoading (LocaleSingleton *_locale) noexcept;

    ModifySingletonQuery modifyLocale;
    InsertLongTermQuery insertLocalizedString;
    RemoveAscendingRangeQuery removeLocalizedString;

    Memory::UniqueString localizationRootPath;
    std::ifstream inputStream;
};

LocalizationSynchronizer::LocalizationSynchronizer (TaskConstructor &_constructor,
                                                    Memory::UniqueString _localizationRootPath) noexcept
    : modifyLocale (MODIFY_SINGLETON (LocaleSingleton)),
      insertLocalizedString (INSERT_LONG_TERM (LocalizedString)),
      removeLocalizedString (REMOVE_ASCENDING_RANGE (LocalizedString, key)),

      localizationRootPath (_localizationRootPath)
{
    _constructor.DependOn (Checkpoint::SYNC_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::SYNC_FINISHED);
}

void LocalizationSynchronizer::Execute () noexcept
{
    auto localeCursor = modifyLocale.Execute ();
    auto *locale = static_cast<LocaleSingleton *> (*localeCursor);
    SyncLocaleRequest (locale);
    UpdateLocaleLoading (locale);
}

void LocalizationSynchronizer::SyncLocaleRequest (LocaleSingleton *_locale) noexcept
{
    if (_locale->targetLocale != _locale->loadedLocale && !*_locale->loadingLocale)
    {
        if (!*_locale->targetLocale)
        {
            EMERGENCE_LOG (ERROR, "Localization: Target locale cannot be empty!");
            return;
        }

        const std::filesystem::path configurationPathBin =
            EMERGENCE_BUILD_STRING (localizationRootPath, "/", _locale->targetLocale, ".bin");

        const std::filesystem::path configurationPathYaml =
            EMERGENCE_BUILD_STRING (localizationRootPath, "/", _locale->targetLocale, ".yaml");

        if (std::filesystem::exists (configurationPathBin))
        {
            inputStream.open (configurationPathBin, std::ios::binary);
            if (!inputStream)
            {
                EMERGENCE_LOG (ERROR, "Localization: Unable to open strings database for locale \"",
                               _locale->targetLocale, "\"!");
                return;
            }

            Job::Dispatcher::Global ().Dispatch (
                Job::Priority::BACKGROUND,
                [this, _locale] ()
                {
                    Serialization::Binary::DeserializeObject (inputStream, &_locale->configurationInLoading,
                                                              LocaleConfiguration::Reflect ().mapping, {});
                    _locale->isConfigurationLoaded.test_and_set (std::memory_order::acquire);
                });
        }
        else if (std::filesystem::exists (configurationPathYaml))
        {
            inputStream.open (configurationPathYaml);
            if (!inputStream)
            {
                EMERGENCE_LOG (ERROR, "Localization: Unable to open strings database for locale \"",
                               _locale->targetLocale, "\"!");
                return;
            }

            Job::Dispatcher::Global ().Dispatch (
                Job::Priority::BACKGROUND,
                [this, _locale] ()
                {
                    Serialization::Yaml::DeserializeObject (inputStream, &_locale->configurationInLoading,
                                                            LocaleConfiguration::Reflect ().mapping, {});
                    _locale->isConfigurationLoaded.test_and_set (std::memory_order::acquire);
                });
        }
        else
        {
            EMERGENCE_LOG (ERROR, "Localization: Unable to find strings database for locale \"", _locale->targetLocale,
                           "\"!");
            return;
        }

        _locale->loadingLocale = _locale->targetLocale;
    }
}

void LocalizationSynchronizer::UpdateLocaleLoading (LocaleSingleton *_locale) noexcept
{
    if (*_locale->loadingLocale && _locale->isConfigurationLoaded.test (std::memory_order::acquire))
    {
        inputStream.close ();

        {
            auto removalCursor = removeLocalizedString.Execute (nullptr, nullptr);
            while (removalCursor.ReadConst ())
            {
                ~removalCursor;
            }
        }

        auto insertionCursor = insertLocalizedString.Execute ();
        for (const LocalizedString &localeString : _locale->configurationInLoading.strings)
        {
            auto *string = static_cast<LocalizedString *> (++insertionCursor);
            string->key = localeString.key;
            string->value = localeString.value;
        }

        _locale->loadedLocale = _locale->loadingLocale;
        _locale->loadingLocale = {};
        _locale->configurationInLoading.strings.clear ();
        _locale->isConfigurationLoaded.clear (std::memory_order::release);
    }
}

void AddToNormalUpdate (PipelineBuilder &_builder, Memory::UniqueString _localizationRootPath) noexcept
{
    _builder.AddCheckpoint (Checkpoint::SYNC_STARTED);
    _builder.AddCheckpoint (Checkpoint::SYNC_FINISHED);
    _builder.AddTask (Memory::UniqueString {"LocalizationSynchronizer"})
        .SetExecutor<LocalizationSynchronizer> (_localizationRootPath);
}
} // namespace Emergence::Celerity::Localization
