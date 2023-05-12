#include <filesystem>
#include <fstream>

#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/Locale/Localization.hpp>
#include <Celerity/Locale/LocalizedString.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity::Localization
{
const Memory::UniqueString Files::STRINGS {"Strings"};

const Memory::UniqueString Checkpoint::SYNC_STARTED {"Localization::SyncStarted"};
const Memory::UniqueString Checkpoint::SYNC_FINISHED {"Localization::SyncFinished"};

class LocalizationSynchronizer final : public TaskExecutorBase<LocalizationSynchronizer>
{
public:
    LocalizationSynchronizer (TaskConstructor &_constructor,
                              Memory::UniqueString _localizationRootPath,
                              uint64_t _maxLoadingTimePerFrameNs) noexcept;

    void Execute () noexcept;

private:
    void SyncLocaleRequest (LocaleSingleton *_locale) noexcept;

    void UpdateLocaleLoading (LocaleSingleton *_locale) noexcept;

    void UpdateLocalizedString (Memory::UniqueString _key, Container::Utf8String _value) noexcept;

    ModifySingletonQuery modifyLocale;
    EditValueQuery editLocalizedString;
    InsertLongTermQuery insertLocalizedString;

    Memory::UniqueString localizationRootPath;
    const uint64_t maxLoadingTimePerFrameNs;

    bool loadingBinary = false;
    std::ifstream inputStream;
    Serialization::Yaml::StringMappingDeserializer yamlStringDeserializer;
};

LocalizationSynchronizer::LocalizationSynchronizer (TaskConstructor &_constructor,
                                                    Memory::UniqueString _localizationRootPath,
                                                    uint64_t _maxLoadingTimePerFrameNs) noexcept
    : modifyLocale (MODIFY_SINGLETON (LocaleSingleton)),
      editLocalizedString (EDIT_VALUE_1F (LocalizedString, key)),
      insertLocalizedString (INSERT_LONG_TERM (LocalizedString)),

      localizationRootPath (_localizationRootPath),
      maxLoadingTimePerFrameNs (_maxLoadingTimePerFrameNs)
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
    if (_locale->targetLocale != _locale->loadedLocale && _locale->targetLocale != _locale->loadingLocale)
    {
        if (*_locale->loadingLocale)
        {
            if (!loadingBinary)
            {
                yamlStringDeserializer.End ();
            }

            inputStream.close ();
        }

        if (!*_locale->targetLocale)
        {
            EMERGENCE_LOG (ERROR, "Localization: Target locale cannot be empty!");
            return;
        }

        const std::filesystem::path stringsPathBin =
            EMERGENCE_BUILD_STRING (localizationRootPath, "/", _locale->targetLocale, "/", Files::STRINGS, ".bin");

        const std::filesystem::path stringsPathYaml =
            EMERGENCE_BUILD_STRING (localizationRootPath, "/", _locale->targetLocale, "/", Files::STRINGS, ".yaml");

        if (std::filesystem::exists (stringsPathBin))
        {
            inputStream.open (stringsPathBin, std::ios::binary);
            if (!inputStream)
            {
                EMERGENCE_LOG (ERROR, "Localization: Unable to open strings database for locale \"",
                               _locale->targetLocale, "\"!");
                return;
            }

            loadingBinary = true;
        }
        else if (std::filesystem::exists (stringsPathYaml))
        {
            inputStream.open (stringsPathYaml);
            if (!inputStream)
            {
                EMERGENCE_LOG (ERROR, "Localization: Unable to open strings database for locale \"",
                               _locale->targetLocale, "\"!");
                return;
            }

            yamlStringDeserializer.Begin (inputStream);
            loadingBinary = false;
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
    if (*_locale->loadingLocale)
    {
        const std::uint64_t startTime = Time::NanosecondsSinceStartup ();
        if (loadingBinary)
        {
            while (startTime + maxLoadingTimePerFrameNs > Time::NanosecondsSinceStartup () && inputStream)
            {
                Container::Utf8String keyString;
                Container::Utf8String value;

                if (!Serialization::Binary::DeserializeString (inputStream, keyString) ||
                    !Serialization::Binary::DeserializeString (inputStream, value))
                {
                    EMERGENCE_LOG (ERROR,
                                   "Localization: IO error occurred during reading string database for locale \"",
                                   _locale->loadingLocale, "\".");
                    break;
                }

                const Memory::UniqueString key {keyString};
                UpdateLocalizedString (key, std::move (value));

                // Use peek to test for the end of file or other problems in given stream.
                inputStream.peek ();
            }

            if (!inputStream)
            {
                inputStream.close ();
                _locale->loadedLocale = _locale->loadingLocale;
                _locale->loadingLocale = {};
            }
        }
        else
        {
            while (startTime + maxLoadingTimePerFrameNs > Time::NanosecondsSinceStartup () &&
                   yamlStringDeserializer.HasNext ())
            {
                Memory::UniqueString key;
                Container::Utf8String value;

                if (!yamlStringDeserializer.Next (key, value))
                {
                    EMERGENCE_LOG (ERROR,
                                   "Localization: IO error occurred during reading string database for locale \"",
                                   _locale->loadingLocale, "\".");
                    break;
                }

                UpdateLocalizedString (key, std::move (value));
                // Use peek to test for the end of file or other problems in given stream.
                inputStream.peek ();
            }

            if (!yamlStringDeserializer.HasNext ())
            {
                yamlStringDeserializer.End ();
                inputStream.close ();
                _locale->loadedLocale = _locale->loadingLocale;
                _locale->loadingLocale = {};
            }
        }
    }
}

void LocalizationSynchronizer::UpdateLocalizedString (Memory::UniqueString _key, Container::Utf8String _value) noexcept
{
    {
        auto cursor = editLocalizedString.Execute (&_key);
        if (auto *string = static_cast<LocalizedString *> (*cursor))
        {
            string->value = std::move (_value);
            return;
        }
    }

    auto cursor = insertLocalizedString.Execute ();
    auto *string = static_cast<LocalizedString *> (++cursor);
    string->key = _key;
    string->value = std::move (_value);
}

void AddToNormalUpdate (PipelineBuilder &_builder,
                        Memory::UniqueString _localizationRootPath,
                        uint64_t _maxLoadingTimePerFrameNs) noexcept
{
    _builder.AddCheckpoint (Checkpoint::SYNC_STARTED);
    _builder.AddCheckpoint (Checkpoint::SYNC_FINISHED);
    _builder.AddTask (Memory::UniqueString {"LocalizationSynchronizer"})
        .SetExecutor<LocalizationSynchronizer> (_localizationRootPath, _maxLoadingTimePerFrameNs);
}
} // namespace Emergence::Celerity::Localization
