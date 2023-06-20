#include <filesystem>
#include <fstream>

#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/Locale/Localization.hpp>
#include <Celerity/Locale/LocalizedString.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Memory::Literals;

static const char *const ENVIRONMENT_ROOT = "Environment";
static const char *const ENVIRONMENT_MOUNT = "Environment";
static const char *const LOCALES_ROOT = "Environment/Locale";

struct LocalizationTestStage
{
    Memory::UniqueString targetLocale;
    LocaleConfiguration expectedConfiguration;
};

class LocalizationTester final : public TaskExecutorBase<LocalizationTester>
{
public:
    LocalizationTester (TaskConstructor &_constructor,
                        Container::Vector<LocalizationTestStage> _stages,
                        bool *_finishedOutput) noexcept;

    void Execute () noexcept;

private:
    ModifySingletonQuery modifyLocale;
    FetchAscendingRangeQuery fetchLocalizedStringByAscendingKey;

    Container::Vector<LocalizationTestStage> stages;
    std::size_t stageIndex = 0u;
    bool *finishedOutput = nullptr;
};

LocalizationTester::LocalizationTester (TaskConstructor &_constructor,
                                        Container::Vector<LocalizationTestStage> _stages,
                                        bool *_finishedOutput) noexcept
    : TaskExecutorBase (_constructor),

      modifyLocale (MODIFY_SINGLETON (LocaleSingleton)),
      fetchLocalizedStringByAscendingKey (FETCH_ASCENDING_RANGE (LocalizedString, key)),

      stages (std::move (_stages)),
      finishedOutput (_finishedOutput)
{
    _constructor.DependOn (Localization::Checkpoint::SYNC_FINISHED);
}

void LocalizationTester::Execute () noexcept
{
    if (stageIndex >= stages.size ())
    {
        *finishedOutput = true;
        return;
    }

    auto localeCursor = modifyLocale.Execute ();
    auto *locale = static_cast<LocaleSingleton *> (*localeCursor);
    locale->targetLocale = stages[stageIndex].targetLocale;
    LOG ("Locale state. Loaded: ", (*locale->loadedLocale ? *locale->loadedLocale : "null"),
         ". Target: ", (*locale->targetLocale ? *locale->targetLocale : "null"),
         ". Loading: ", (*locale->loadingLocale ? *locale->loadingLocale : "null"), ".");

    if (locale->loadedLocale == locale->targetLocale)
    {
        std::size_t stringIndex = 0u;
        for (auto cursor = fetchLocalizedStringByAscendingKey.Execute (nullptr, nullptr);
             const auto *string = static_cast<const LocalizedString *> (*cursor); ++stringIndex, ++cursor)
        {
            // We expect that test strings are sorted by their id.
            if (stringIndex < stages[stageIndex].expectedConfiguration.strings.size ())
            {
                CHECK_EQUAL (stages[stageIndex].expectedConfiguration.strings[stringIndex].key, string->key);
                CHECK_EQUAL (stages[stageIndex].expectedConfiguration.strings[stringIndex].value, string->value);
            }
        }

        CHECK_EQUAL (stringIndex, stages[stageIndex].expectedConfiguration.strings.size ());
        LOG ("Finished stage ", stageIndex, ".");
        ++stageIndex;
    }
}

struct EnvironmentLocale
{
    Memory::UniqueString locale;
    bool binary = false;
    LocaleConfiguration configuration;
};

using Environment = Container::Vector<EnvironmentLocale>;

VirtualFileSystem::Context PrepareEnvironment (const Environment &_environment)
{
    const std::filesystem::path rootPath {ENVIRONMENT_ROOT};
    if (std::filesystem::exists (rootPath))
    {
        std::filesystem::remove_all (rootPath);
    }

    std::filesystem::create_directories (ENVIRONMENT_ROOT);
    VirtualFileSystem::Context virtualFileSystem;
    REQUIRE (virtualFileSystem.Mount (virtualFileSystem.GetRoot (), {VirtualFileSystem::MountSource::FILE_SYSTEM,
                                                                     ENVIRONMENT_ROOT, ENVIRONMENT_MOUNT}));
    std::filesystem::create_directories (LOCALES_ROOT);

    for (const EnvironmentLocale &locale : _environment)
    {
        if (locale.binary)
        {
            const std::filesystem::path stringsPath {EMERGENCE_BUILD_STRING (*locale.locale, ".bin")};
            std::ofstream output {LOCALES_ROOT / stringsPath, std::ios::binary};
            Serialization::Binary::SerializeTypeName (output, LocaleConfiguration::Reflect ().mapping.GetName ());
            Serialization::Binary::SerializeObject (output, &locale.configuration,
                                                    LocaleConfiguration::Reflect ().mapping);
        }
        else
        {
            const std::filesystem::path stringsPath {EMERGENCE_BUILD_STRING (*locale.locale, ".yaml")};
            std::ofstream output {LOCALES_ROOT / stringsPath, std::ios::binary};
            Serialization::Yaml::SerializeTypeName (output, LocaleConfiguration::Reflect ().mapping.GetName ());
            Serialization::Yaml::SerializeObject (output, &locale.configuration,
                                                  LocaleConfiguration::Reflect ().mapping);
        }
    }

    return virtualFileSystem;
}

void ExecuteTest (const Environment &_environment, Container::Vector<LocalizationTestStage> _stages)
{
    VirtualFileSystem::Context virtualFileSystem = PrepareEnvironment (_environment);
    bool testFinished = false;
    std::size_t frameIndex = 0u;

    World world {"TestWorld"_us, WorldConfiguration {}};
    PipelineBuilder builder {world.GetRootView ()};

    Container::MappingRegistry resourceTypeRegistry;
    resourceTypeRegistry.Register (LocaleConfiguration::Reflect ().mapping);
    Resource::Provider::ResourceProvider resourceProvider {&virtualFileSystem, resourceTypeRegistry, {}};
    REQUIRE ((resourceProvider.AddSource (Memory::UniqueString {ENVIRONMENT_MOUNT}) ==
              Resource::Provider::SourceOperationResponse::SUCCESSFUL));

    builder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
    Localization::AddToNormalUpdate (builder, &resourceProvider);
    builder.AddTask ("LocalizationTester"_us).SetExecutor<LocalizationTester> (std::move (_stages), &testFinished);
    REQUIRE (builder.End ());

    while (!testFinished)
    {
        world.Update ();
        ++frameIndex;
        REQUIRE ((frameIndex < 1000u));
    }
}

static const EnvironmentLocale ENGLISH_NORMAL {"EnglishNormal"_us,
                                               false,
                                               {{
                                                   {"Quit"_us, "Quit"},
                                                   {"Settings"_us, "Settings"},
                                                   {"StartCampaign"_us, "Start campaign"},
                                                   {"StartTutorial"_us, "Start tutorial"},
                                               }}};

static const EnvironmentLocale ENGLISH_STRANGE {"EnglishStrange"_us,
                                                true,
                                                {{
                                                    {"Quit"_us, "Quott"},
                                                    {"Settings"_us, "Sotongus"},
                                                    {"StartCampaign"_us, "Stord compgn"},
                                                    {"StartTutorial"_us, "Stord ttrl"},
                                                }}};
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity::Test;

BEGIN_SUITE (Localization)

TEST_CASE (Yaml)
{
    ExecuteTest ({ENGLISH_NORMAL}, {{ENGLISH_NORMAL.locale, ENGLISH_NORMAL.configuration}});
}

TEST_CASE (Binary)
{
    ExecuteTest ({ENGLISH_STRANGE}, {{ENGLISH_STRANGE.locale, ENGLISH_STRANGE.configuration}});
}

TEST_CASE (Reload)
{
    ExecuteTest ({ENGLISH_NORMAL, ENGLISH_STRANGE}, {{ENGLISH_NORMAL.locale, ENGLISH_NORMAL.configuration},
                                                     {ENGLISH_STRANGE.locale, ENGLISH_STRANGE.configuration}});
}

END_SUITE
