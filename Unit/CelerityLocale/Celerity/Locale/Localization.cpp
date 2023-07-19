#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/Locale/Localization.hpp>
#include <Celerity/Locale/LocalizedString.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::Localization
{
using namespace Memory::Literals;

const Memory::UniqueString Checkpoint::SYNC_STARTED {"Localization::SyncStarted"};
const Memory::UniqueString Checkpoint::SYNC_FINISHED {"Localization::SyncFinished"};

class LocalizationSynchronizer final : public TaskExecutorBase<LocalizationSynchronizer>
{
public:
    LocalizationSynchronizer (TaskConstructor &_constructor,
                              Resource::Provider::ResourceProvider *_resourceProvider) noexcept;

    void Execute () noexcept;

private:
    void SyncLocaleRequest (LocaleSingleton *_locale) noexcept;

    void UpdateLocaleLoading (LocaleSingleton *_locale) noexcept;

    ModifySingletonQuery modifyLocale;
    InsertLongTermQuery insertLocalizedString;
    RemoveAscendingRangeQuery removeLocalizedString;

    Resource::Provider::ResourceProvider *resourceProvider;
};

LocalizationSynchronizer::LocalizationSynchronizer (TaskConstructor &_constructor,
                                                    Resource::Provider::ResourceProvider *_resourceProvider) noexcept
    : TaskExecutorBase (_constructor),

      modifyLocale (MODIFY_SINGLETON (LocaleSingleton)),
      insertLocalizedString (INSERT_LONG_TERM (LocalizedString)),
      removeLocalizedString (REMOVE_ASCENDING_RANGE (LocalizedString, key)),

      resourceProvider (_resourceProvider)
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

        _locale->sharedState->loadingState = LocaleLoadingState::LOADING;
        Job::Dispatcher::Global ().Dispatch (
            Job::Priority::BACKGROUND,
            [targetLocale {_locale->targetLocale}, capturedResourceProvider {resourceProvider},
             sharedState {_locale->sharedState}] ()
            {
                static CPU::Profiler::SectionDefinition loadingSection {*"LocalizationLoading"_us, 0xFF999900u};
                CPU::Profiler::SectionInstance section {loadingSection};

                switch (capturedResourceProvider->LoadObject (LocaleConfiguration::Reflect ().mapping, targetLocale,
                                                              &sharedState->configurationInLoading))
                {
                case Resource::Provider::LoadingOperationResponse::SUCCESSFUL:
                    sharedState->loadingState = LocaleLoadingState::SUCCESSFUL;
                    break;

                case Resource::Provider::LoadingOperationResponse::NOT_FOUND:
                    EMERGENCE_LOG (ERROR, "Localization: Unable to find locale \"", targetLocale, "\"!");
                    sharedState->loadingState = LocaleLoadingState::FAILED;
                    break;

                case Resource::Provider::LoadingOperationResponse::IO_ERROR:
                case Resource::Provider::LoadingOperationResponse::WRONG_TYPE:
                    EMERGENCE_LOG (ERROR, "Localization: Unable to read locale \"", targetLocale, "\"!");
                    sharedState->loadingState = LocaleLoadingState::FAILED;
                    break;
                }
            });

        _locale->loadingLocale = _locale->targetLocale;
    }
}

void LocalizationSynchronizer::UpdateLocaleLoading (LocaleSingleton *_locale) noexcept
{
    if (*_locale->loadingLocale && _locale->sharedState->loadingState != LocaleLoadingState::LOADING)
    {
        if (_locale->sharedState->loadingState == LocaleLoadingState::SUCCESSFUL)
        {
            {
                auto removalCursor = removeLocalizedString.Execute (nullptr, nullptr);
                while (removalCursor.ReadConst ())
                {
                    ~removalCursor;
                }
            }

            auto insertionCursor = insertLocalizedString.Execute ();
            for (const LocalizedString &localeString : _locale->sharedState->configurationInLoading.strings)
            {
                auto *string = static_cast<LocalizedString *> (++insertionCursor);
                string->key = localeString.key;
                string->value = localeString.value;
            }

            _locale->loadedLocale = _locale->loadingLocale;
            _locale->loadingLocale = {};
            _locale->sharedState->configurationInLoading.strings.clear ();
        }
        else
        {
            EMERGENCE_LOG (ERROR, "Localization: Unable to load locale \"", _locale->loadingLocale,
                           "\", falling back to \"", _locale->loadedLocale, "\"!");
            _locale->loadingLocale = {};
            _locale->targetLocale = _locale->loadedLocale;
        }
    }
}

void AddToNormalUpdate (PipelineBuilder &_builder, Resource::Provider::ResourceProvider *_resourceProvider) noexcept
{
    _builder.AddCheckpoint (Checkpoint::SYNC_STARTED);
    _builder.AddCheckpoint (Checkpoint::SYNC_FINISHED);
    _builder.AddTask ("LocalizationSynchronizer"_us).SetExecutor<LocalizationSynchronizer> (_resourceProvider);
}
} // namespace Emergence::Celerity::Localization