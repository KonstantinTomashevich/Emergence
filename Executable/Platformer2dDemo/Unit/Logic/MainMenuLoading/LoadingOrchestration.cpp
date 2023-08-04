#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/UI/UIRenderPass.hpp>
#include <Celerity/WorldSingleton.hpp>

#include <Configuration/Localization.hpp>
#include <Configuration/VisibilityMask.hpp>

#include <GameCore/GameStateSingleton.hpp>
#include <GameCore/LevelsConfigurationSingleton.hpp>

#include <LevelLoading/LevelLoading.hpp>
#include <LevelLoading/LevelLoadingSingleton.hpp>

#include <LoadingAnimation/LoadingAnimation.hpp>
#include <LoadingAnimation/LoadingAnimationSingleton.hpp>

#include <MainMenuLoading/LoadingOrchestration.hpp>
#include <MainMenuLoading/MainMenuLoadingSingleton.hpp>

#include <Render/Backend/Configuration.hpp>

namespace MainMenuLoadingOrchestration
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"MainMenuLoadingOrchestration::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"MainMenuLoadingOrchestration::Finished"};

class LoadingOrchestrator final : public Emergence::Celerity::TaskExecutorBase<LoadingOrchestrator>
{
public:
    LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void SpawnViewports () noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchAssetManager;
    Emergence::Celerity::FetchSingletonQuery fetchLevelsConfiguration;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;

    Emergence::Celerity::ModifySingletonQuery modifyGameState;
    Emergence::Celerity::ModifySingletonQuery modifyLevelLoading;
    Emergence::Celerity::ModifySingletonQuery modifyLoadingAnimation;
    Emergence::Celerity::ModifySingletonQuery modifyLocale;
    Emergence::Celerity::ModifySingletonQuery modifyMainMenuLoading;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;
    Emergence::Celerity::InsertLongTermQuery insertUIPass;
};

LoadingOrchestrator::LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      fetchAssetManager (FETCH_SINGLETON (Emergence::Celerity::AssetManagerSingleton)),
      fetchLevelsConfiguration (FETCH_SINGLETON (LevelsConfigurationSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      modifyGameState (MODIFY_SINGLETON (GameStateSingleton)),
      modifyLevelLoading (MODIFY_SINGLETON (LevelLoadingSingleton)),
      modifyLoadingAnimation (MODIFY_SINGLETON (LoadingAnimationSingleton)),
      modifyLocale (MODIFY_SINGLETON (Emergence::Celerity::LocaleSingleton)),
      modifyMainMenuLoading (MODIFY_SINGLETON (MainMenuLoadingSingleton)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),
      insertUIPass (INSERT_LONG_TERM (Emergence::Celerity::UIRenderPass))
{
    _constructor.DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (LevelLoading::Checkpoint::STARTED);
    _constructor.MakeDependencyOf (LoadingAnimation::Checkpoint::STARTED);
}

void LoadingOrchestrator::Execute () noexcept
{
    auto gameStateCursor = modifyGameState.Execute ();
    auto *gameState = static_cast<GameStateSingleton *> (*gameStateCursor);

    if (gameState->state != GameState::MAIN_MENU_LOADING)
    {
        // Waiting for transition to happen.
        return;
    }

    auto levelLoadingCursor = modifyLevelLoading.Execute ();
    auto *levelLoading = static_cast<LevelLoadingSingleton *> (*levelLoadingCursor);

    if (!*levelLoading->levelName)
    {
        levelLoading->levelName = Emergence::Memory::UniqueString {"RO_MainMenu"};
    }

    auto localeCursor = modifyLocale.Execute ();
    auto *locale = static_cast<Emergence::Celerity::LocaleSingleton *> (*localeCursor);

    if (!*locale->targetLocale)
    {
        locale->targetLocale = Localization::HARDCODED_LOCALE;
    }

    auto levelsConfigurationCursor = fetchLevelsConfiguration.Execute ();
    const auto *levelsConfiguration = static_cast<const LevelsConfigurationSingleton *> (*levelsConfigurationCursor);

    auto mainMenuLoadingCursor = modifyMainMenuLoading.Execute ();
    auto *mainMenuLoading = static_cast<MainMenuLoadingSingleton *> (*mainMenuLoadingCursor);

    auto assetManagerCursor = fetchAssetManager.Execute ();
    const auto *assetManager = static_cast<const Emergence::Celerity::AssetManagerSingleton *> (*assetManagerCursor);
    const bool assetsWereLoaded = mainMenuLoading->assetsLoaded;
    mainMenuLoading->assetsLoaded = assetManager->assetsLeftToLoad == 0u;

    auto loadingAnimationCursor = modifyLoadingAnimation.Execute ();
    auto *loadingAnimation = static_cast<LoadingAnimationSingleton *> (*loadingAnimationCursor);

    if (levelsConfiguration->loaded && levelLoading->state == LevelLoadingState::DONE && assetsWereLoaded &&
        mainMenuLoading->assetsLoaded && locale->loadedLocale == locale->targetLocale)
    {
        loadingAnimation->required = false;
        SpawnViewports ();
        gameState->request.state = GameState::MAIN_MENU;
    }
    else
    {
        loadingAnimation->required = true;
    }
}

void LoadingOrchestrator::SpawnViewports () noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto transformCursor = insertTransform.Execute ();
    auto *cameraTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
    cameraTransform->SetObjectId (world->GenerateId ());
    cameraTransform->SetVisualLocalTransform ({{0.0f, 0.0f}, 0.0f, {1.0f, 1.0f}});

    auto cameraCursor = insertCamera.Execute ();
    auto *camera = static_cast<Emergence::Celerity::Camera2dComponent *> (++cameraCursor);
    camera->objectId = cameraTransform->GetObjectId ();
    camera->halfOrthographicSize = 3.75f;
    camera->visibilityMask = static_cast<std::uint64_t> (VisibilityMask::GAME_SCENE);

    auto viewportCursor = insertViewport.Execute ();
    auto *worldViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);

    worldViewport->name = "MainMenuScene"_us;
    worldViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
    worldViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
    worldViewport->clearColor = 0x000000FF;
    worldViewport->sortIndex = 0u;

    auto worldPassCursor = insertWorldPass.Execute ();
    auto *worldPass = static_cast<Emergence::Celerity::World2dRenderPass *> (++worldPassCursor);
    worldPass->name = worldViewport->name;
    worldPass->cameraObjectId = camera->objectId;

    auto *uiViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);
    uiViewport->name = "MainMenuUI"_us;
    uiViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
    uiViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
    uiViewport->clearColor = 0x00000000;
    uiViewport->sortIndex = 1u;

    auto uiPassCursor = insertUIPass.Execute ();
    auto *uiPass = static_cast<Emergence::Celerity::UIRenderPass *> (++uiPassCursor);
    uiPass->name = uiViewport->name;
    uiPass->defaultStyleId = "Default"_us;
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"MainMenuLoadingOrchestrator"})
        .SetExecutor<LoadingOrchestrator> ();
}
} // namespace MainMenuLoadingOrchestration
