#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Configuration/Paths.hpp>

#include <Framework/GameState.hpp>

#include <LoadingAnimation/LoadingAnimation.hpp>
#include <LoadingAnimation/LoadingAnimationSingleton.hpp>

#include <MainMenuLoading/LevelsConfigurationLoading.hpp>
#include <MainMenuLoading/LoadingOrchestration.hpp>
#include <MainMenuLoading/MainMenuLoadingSingleton.hpp>
#include <MainMenuLoading/MainMenuInitialization.hpp>

namespace MainMenuLoadingOrchestration
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"MainMenuLoadingOrchestration::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"MainMenuLoadingOrchestration::Finished"};

class LoadingOrchestrator final : public Emergence::Celerity::TaskExecutorBase<LoadingOrchestrator>
{
public:
    LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor,
                         const ViewDropHandle &_viewDropHandle,
                         Emergence::Celerity::WorldView *_ownerView) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyWorld;
    Emergence::Celerity::ModifySingletonQuery modifyMainMenuLoading;
    Emergence::Celerity::FetchSingletonQuery fetchAssetManager;
    Emergence::Celerity::ModifySingletonQuery modifyLoadingAnimation;

    ViewDropHandle viewDropHandle;
    Emergence::Celerity::WorldView *ownerView;
};

LoadingOrchestrator::LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor,
                                          const ViewDropHandle &_viewDropHandle,
                                          Emergence::Celerity::WorldView *_ownerView) noexcept
    : modifyWorld (MODIFY_SINGLETON (Emergence::Celerity::WorldSingleton)),
      modifyMainMenuLoading (MODIFY_SINGLETON (MainMenuLoadingSingleton)),
      fetchAssetManager (FETCH_SINGLETON (Emergence::Celerity::AssetManagerSingleton)),
      modifyLoadingAnimation (MODIFY_SINGLETON (LoadingAnimationSingleton)),

      viewDropHandle (_viewDropHandle),
      ownerView (_ownerView)
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (MainMenuInitialization::Checkpoint::STARTED);
    _constructor.MakeDependencyOf (LoadingAnimation::Checkpoint::STARTED);
}

void LoadingOrchestrator::Execute () noexcept
{
    auto worldCursor = modifyWorld.Execute ();
    auto *world = static_cast<Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto mainMenuLoadingCursor = modifyMainMenuLoading.Execute ();
    auto *mainMenuLoading = static_cast<MainMenuLoadingSingleton *> (*mainMenuLoadingCursor);

    auto assetManagerCursor = fetchAssetManager.Execute ();
    const auto *assetManager = static_cast<const Emergence::Celerity::AssetManagerSingleton *> (*assetManagerCursor);
    mainMenuLoading->assetsLoaded = assetManager->assetsLeftToLoad == 0u;

    auto loadingAnimationCursor = modifyLoadingAnimation.Execute ();
    auto *loadingAnimation = static_cast<LoadingAnimationSingleton *> (*loadingAnimationCursor);

    if (mainMenuLoading->levelsConfigurationLoaded && mainMenuLoading->mainMenuInitialized &&
        mainMenuLoading->assetsLoaded)
    {
        loadingAnimation->required = false;
        world->updateMode = Emergence::Celerity::WorldUpdateMode::SIMULATING;
        viewDropHandle.RequestViewDrop (ownerView);
    }
    else
    {
        loadingAnimation->required = true;
        world->updateMode = Emergence::Celerity::WorldUpdateMode::FROZEN;
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        const ViewDropHandle &_viewDropHandle,
                        Emergence::Celerity::WorldView *_ownerView) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"MainMenuLoadingOrchestrator"})
        .SetExecutor<LoadingOrchestrator> (_viewDropHandle, _ownerView);
}
} // namespace MainMenuLoadingOrchestration
