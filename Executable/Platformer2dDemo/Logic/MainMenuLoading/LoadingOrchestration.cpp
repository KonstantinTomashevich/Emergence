#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/UI/UIRenderPass.hpp>

#include <Configuration/Localization.hpp>
#include <Configuration/VisibilityMask.hpp>

#include <Framework/GameState.hpp>

#include <LevelLoading/LevelLoading.hpp>
#include <LevelLoading/LevelLoadingSingleton.hpp>

#include <LoadingAnimation/LoadingAnimation.hpp>
#include <LoadingAnimation/LoadingAnimationSingleton.hpp>

#include <MainMenuLoading/LoadingOrchestration.hpp>
#include <MainMenuLoading/MainMenuLoadingSingleton.hpp>

#include <Render/Backend/Configuration.hpp>

#include <Root/LevelsConfigurationSingleton.hpp>

namespace MainMenuLoadingOrchestration
{
using namespace Emergence::Memory::Literals;

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
    void SpawnViewports (const Emergence::Celerity::WorldSingleton *_world) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchAssetManager;
    Emergence::Celerity::FetchSingletonQuery fetchLevelsConfiguration;
    Emergence::Celerity::ModifySingletonQuery modifyLevelLoading;
    Emergence::Celerity::ModifySingletonQuery modifyLoadingAnimation;
    Emergence::Celerity::ModifySingletonQuery modifyLocale;
    Emergence::Celerity::ModifySingletonQuery modifyMainMenuLoading;
    Emergence::Celerity::ModifySingletonQuery modifyWorld;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;
    Emergence::Celerity::InsertLongTermQuery insertUIPass;

    ViewDropHandle viewDropHandle;
    Emergence::Celerity::WorldView *ownerView;
};

LoadingOrchestrator::LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor,
                                          const ViewDropHandle &_viewDropHandle,
                                          Emergence::Celerity::WorldView *_ownerView) noexcept
    : fetchAssetManager (FETCH_SINGLETON (Emergence::Celerity::AssetManagerSingleton)),
      fetchLevelsConfiguration (FETCH_SINGLETON (LevelsConfigurationSingleton)),
      modifyLevelLoading (MODIFY_SINGLETON (LevelLoadingSingleton)),
      modifyLoadingAnimation (MODIFY_SINGLETON (LoadingAnimationSingleton)),
      modifyLocale (MODIFY_SINGLETON (Emergence::Celerity::LocaleSingleton)),
      modifyMainMenuLoading (MODIFY_SINGLETON (MainMenuLoadingSingleton)),
      modifyWorld (MODIFY_SINGLETON (Emergence::Celerity::WorldSingleton)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),
      insertUIPass (INSERT_LONG_TERM (Emergence::Celerity::UIRenderPass)),

      viewDropHandle (_viewDropHandle),
      ownerView (_ownerView)
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (LevelLoading::Checkpoint::STARTED);
    _constructor.MakeDependencyOf (LoadingAnimation::Checkpoint::STARTED);
}

void LoadingOrchestrator::Execute () noexcept
{
    auto worldCursor = modifyWorld.Execute ();
    auto *world = static_cast<Emergence::Celerity::WorldSingleton *> (*worldCursor);

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
        world->updateMode = Emergence::Celerity::WorldUpdateMode::SIMULATING;
        SpawnViewports (world);
        viewDropHandle.RequestViewDrop (ownerView);
    }
    else
    {
        loadingAnimation->required = true;
        world->updateMode = Emergence::Celerity::WorldUpdateMode::FROZEN;
    }
}

void LoadingOrchestrator::SpawnViewports (const Emergence::Celerity::WorldSingleton *_world) noexcept
{
    auto transformCursor = insertTransform.Execute ();
    auto *cameraTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
    cameraTransform->SetObjectId (_world->GenerateId ());
    cameraTransform->SetVisualLocalTransform ({{0.0f, 0.0f}, 0.0f, {1.0f, 1.0f}});

    auto cameraCursor = insertCamera.Execute ();
    auto *camera = static_cast<Emergence::Celerity::Camera2dComponent *> (++cameraCursor);
    camera->objectId = cameraTransform->GetObjectId ();
    camera->halfOrthographicSize = 3.75f;
    camera->visibilityMask = static_cast<uint64_t> (VisibilityMask::GAME_SCENE);

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
