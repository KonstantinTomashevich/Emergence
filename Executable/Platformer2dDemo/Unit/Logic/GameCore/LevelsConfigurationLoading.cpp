#include <Celerity/PipelineBuilderMacros.hpp>

#include <GameCore/LevelsConfigurationLoading.hpp>
#include <GameCore/LevelsConfigurationSingleton.hpp>

namespace LevelsConfigurationLoading
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"LevelsConfigurationLoading::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"LevelsConfigurationLoading::Finished"};

class Loader final : public Emergence::Celerity::TaskExecutorBase<Loader>
{
public:
    Loader (Emergence::Celerity::TaskConstructor &_constructor,
            Emergence::Resource::Provider::ResourceProvider *_resourceProvider) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyLevelsConfiguration;
    Emergence::Resource::Provider::ResourceProvider *resourceProvider;
};

Loader::Loader (Emergence::Celerity::TaskConstructor &_constructor,
                Emergence::Resource::Provider::ResourceProvider *_resourceProvider) noexcept
    : TaskExecutorBase (_constructor),

      modifyLevelsConfiguration (MODIFY_SINGLETON (LevelsConfigurationSingleton)),
      resourceProvider (_resourceProvider)
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (Checkpoint::FINISHED);
}

void Loader::Execute () noexcept
{
    auto levelsConfigurationCursor = modifyLevelsConfiguration.Execute ();
    auto *levelsConfiguration = static_cast<LevelsConfigurationSingleton *> (*levelsConfigurationCursor);

    if (!levelsConfiguration->loaded)
    {
        // Levels configuration is pretty lightweight, so we're loading it in one frame.
        switch (resourceProvider->LoadObject (LevelsConfigurationSingleton::Reflect ().mapping, "LCS_Levels"_us,
                                              levelsConfiguration))
        {
        case Emergence::Resource::Provider::LoadingOperationResponse::SUCCESSFUL:
            break;

        case Emergence::Resource::Provider::LoadingOperationResponse::NOT_FOUND:
            Emergence::ReportCriticalError ("LevelsConfigurationLoading: Levels configuration not found!", __FILE__,
                                            __LINE__);
            break;

        case Emergence::Resource::Provider::LoadingOperationResponse::IO_ERROR:
            Emergence::ReportCriticalError ("LevelsConfigurationLoading: Unable to deserialize levels configuration!",
                                            __FILE__, __LINE__);
            break;

        case Emergence::Resource::Provider::LoadingOperationResponse::WRONG_TYPE:
            Emergence::ReportCriticalError ("LevelsConfigurationLoading: Levels configuration has wrong type!",
                                            __FILE__, __LINE__);
            break;
        }

        levelsConfiguration->loaded = true;
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        Emergence::Resource::Provider::ResourceProvider *_resourceProvider) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"LevelsConfigurationLoader"})
        .SetExecutor<Loader> (_resourceProvider);
}
} // namespace LevelsConfigurationLoading
