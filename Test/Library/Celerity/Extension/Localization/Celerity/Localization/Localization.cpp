#include <filesystem>
#include <fstream>

#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/Locale/Localization.hpp>
#include <Celerity/Locale/LocalizedString.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Memory::Literals;

static const char *const ENVIRONMENT_ROOT = "Environment";
static const char *const LOCALES_ROOT = "Environment/Locale";

struct LocalizationTestStage
{
    Memory::UniqueString targetLocale;
    Container::Vector<LocalizedString> expectedStrings;
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
    size_t stageIndex = 0u;
    bool *finishedOutput = nullptr;
};

LocalizationTester::LocalizationTester (TaskConstructor &_constructor,
                                        Container::Vector<LocalizationTestStage> _stages,
                                        bool *_finishedOutput) noexcept
    : modifyLocale (MODIFY_SINGLETON (LocaleSingleton)),
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

    if (locale->loadedLocale == locale->targetLocale)
    {
        size_t stringIndex = 0u;
        for (auto cursor = fetchLocalizedStringByAscendingKey.Execute (nullptr, nullptr);
             const auto *string = static_cast<const LocalizedString *> (*cursor); ++stringIndex, ++cursor)
        {
            // We expect that test strings are sorted by their id.
            if (stringIndex < stages[stageIndex].expectedStrings.size ())
            {
                CHECK_EQUAL (stages[stageIndex].expectedStrings[stringIndex].key, string->key);
                CHECK_EQUAL (stages[stageIndex].expectedStrings[stringIndex].value, string->value);
            }
        }

        CHECK_EQUAL (stringIndex, stages[stageIndex].expectedStrings.size ());
        ++stageIndex;
    }
}

struct EnvironmentLocale
{
    Memory::UniqueString locale;
    bool binary = false;
    Container::Vector<LocalizedString> strings;
};

using Environment = Container::Vector<EnvironmentLocale>;

void PrepareEnvironment (const Environment &_environment)
{
    const std::filesystem::path rootPath {ENVIRONMENT_ROOT};
    if (std::filesystem::exists (rootPath))
    {
        std::filesystem::remove_all (rootPath);
    }

    for (const EnvironmentLocale &locale : _environment)
    {
        const std::filesystem::path localePath {EMERGENCE_BUILD_STRING (LOCALES_ROOT, "/", locale.locale, "/")};
        std::filesystem::create_directories (localePath);

        if (locale.binary)
        {
            const std::filesystem::path stringsPath {EMERGENCE_BUILD_STRING (Localization::Files::STRINGS, ".bin")};
            std::ofstream output {localePath / stringsPath, std::ios::binary};

            for (const LocalizedString &string : locale.strings)
            {
                Serialization::Binary::SerializeString (output, *string.key);
                Serialization::Binary::SerializeString (output, string.value.c_str ());
            }
        }
        else
        {
            const std::filesystem::path stringsPath {EMERGENCE_BUILD_STRING (Localization::Files::STRINGS, ".yaml")};
            std::ofstream output {localePath / stringsPath};
            Serialization::Yaml::StringMappingSerializer serializer;
            serializer.Begin ();

            for (const LocalizedString &string : locale.strings)
            {
                serializer.Next (string.key, string.value);
            }

            serializer.End (output);
        }
    }
}

void ExecuteTest (const Environment &_environment, Container::Vector<LocalizationTestStage> _stages)
{
    PrepareEnvironment (_environment);
    bool testFinished = false;
    std::size_t frameIndex = 0u;

    World world {"TestWorld"_us, WorldConfiguration {}};
    PipelineBuilder builder {world.GetRootView ()};

    builder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
    Localization::AddToNormalUpdate (builder, Memory::UniqueString {LOCALES_ROOT}, 1000000u);
    builder.AddTask ("LocalizationTester"_us).SetExecutor<LocalizationTester> (std::move (_stages), &testFinished);
    REQUIRE (builder.End ());

    while (!testFinished)
    {
        world.Update ();
        ++frameIndex;
        REQUIRE (frameIndex < 1000u);
    }
}

static const EnvironmentLocale ENGLISH_NORMAL {"EnglishNormal"_us,
                                               false,
                                               {
                                                   {"Quit"_us, "Quit"},
                                                   {"Settings"_us, "Settings"},
                                                   {"StartCampaign"_us, "Start campaign"},
                                                   {"StartTutorial"_us, "Start tutorial"},
                                               }};

static const EnvironmentLocale ENGLISH_STRANGE {"EnglishStrange"_us,
                                                true,
                                                {
                                                    {"Quit"_us, "Quott"},
                                                    {"Settings"_us, "Sotongus"},
                                                    {"StartCampaign"_us, "Stord compgn"},
                                                    {"StartTutorial"_us, "Stord ttrl"},
                                                }};
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity::Test;

BEGIN_SUITE (Localization)

TEST_CASE (Yaml)
{
    ExecuteTest ({ENGLISH_NORMAL}, {{ENGLISH_NORMAL.locale, ENGLISH_NORMAL.strings}});
}

TEST_CASE (Binary)
{
    ExecuteTest ({ENGLISH_STRANGE}, {{ENGLISH_STRANGE.locale, ENGLISH_STRANGE.strings}});
}

TEST_CASE (Reload)
{
    ExecuteTest ({ENGLISH_NORMAL, ENGLISH_STRANGE},
                 {{ENGLISH_NORMAL.locale, ENGLISH_NORMAL.strings}, {ENGLISH_STRANGE.locale, ENGLISH_STRANGE.strings}});
}

END_SUITE
