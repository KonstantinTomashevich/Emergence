#include <fstream>

#include <Celerity/PipelineBuilderMacros.hpp>

#include <Configuration/Paths.hpp>

#include <MainMenuLoading/LevelsConfigurationLoading.hpp>
#include <MainMenuLoading/MainMenuLoadingSingleton.hpp>

#include <Root/LevelsConfigurationSingleton.hpp>

#include <Serialization/Yaml.hpp>

namespace LevelsConfigurationLoading
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"LevelsConfigurationLoading::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"LevelsConfigurationLoading::Finished"};

class Loader final : public Emergence::Celerity::TaskExecutorBase<Loader>
{
public:
    Loader (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyMainMenuLoading;
    Emergence::Celerity::ModifySingletonQuery modifyLevelsConfiguration;

    Emergence::Serialization::FieldNameLookupCache levelsConfigurationLoadingCache {
        LevelsConfigurationSingleton::Reflect ().mapping};
};

Loader::Loader (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyMainMenuLoading (MODIFY_SINGLETON (MainMenuLoadingSingleton)),
      modifyLevelsConfiguration (MODIFY_SINGLETON (LevelsConfigurationSingleton))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (Checkpoint::FINISHED);
}

void Loader::Execute () noexcept
{
    auto mainMenuLoadingCursor = modifyMainMenuLoading.Execute ();
    auto *mainMenuLoading = static_cast<MainMenuLoadingSingleton *> (*mainMenuLoadingCursor);

    if (!mainMenuLoading->levelsConfigurationLoaded)
    {
        auto levelsConfigurationCursor = modifyLevelsConfiguration.Execute ();
        auto *levelsConfiguration = static_cast<LevelsConfigurationSingleton *> (*levelsConfigurationCursor);

        if (!*levelsConfiguration->tutorialLevelName)
        {
            // Levels configuration is pretty lightweight, so we're loading it in one frame.
            std::ifstream input {*GetLevelsConfigurationPath ()};

            if (input)
            {
                if (!Emergence::Serialization::Yaml::DeserializeObject (input, levelsConfiguration,
                                                                        levelsConfigurationLoadingCache))
                {
                    Emergence::ReportCriticalError (
                        "LevelsConfigurationLoading: Unable to deserialize levels configuration!", __FILE__, __LINE__);
                }

                EMERGENCE_ASSERT (*levelsConfiguration->tutorialLevelName);
            }
            else
            {
                Emergence::ReportCriticalError ("LevelsConfigurationLoading: Unable to open levels configuration!",
                                                __FILE__, __LINE__);
            }
        }

        mainMenuLoading->levelsConfigurationLoaded = true;
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"LevelsConfigurationLoader"}).SetExecutor<Loader> ();
}
} // namespace LevelsConfigurationLoading