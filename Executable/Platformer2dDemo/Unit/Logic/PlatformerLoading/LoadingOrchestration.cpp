#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Input/InputTriggers.hpp>
#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Resource/Config/Messages.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
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

#include <Log/Log.hpp>

#include <Platformer/Animation/CharacterAnimationConfiguration.hpp>
#include <Platformer/Camera/CameraContextComponent.hpp>
#include <Platformer/Input/InputActions.hpp>
#include <Platformer/Movement/MovementConfiguration.hpp>

#include <PlatformerLoading/LoadingOrchestration.hpp>
#include <PlatformerLoading/PlatformerLoadingSingleton.hpp>

#include <Render/Backend/Configuration.hpp>

#include <Time/Time.hpp>

namespace PlatformerLoadingOrchestration
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"PlatformerLoadingOrchestration::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"PlatformerLoadingOrchestration::Finished"};

class LoadingOrchestrator final : public Emergence::Celerity::TaskExecutorBase<LoadingOrchestrator>
{
public:
    LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void SpawnViewports () noexcept;

    void InitInput () noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchAssetManager;
    Emergence::Celerity::FetchSingletonQuery fetchLevelsConfiguration;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::ModifySingletonQuery modifyGameState;
    Emergence::Celerity::ModifySingletonQuery modifyLevelLoading;
    Emergence::Celerity::ModifySingletonQuery modifyLoadingAnimation;
    Emergence::Celerity::ModifySingletonQuery modifyLocale;
    Emergence::Celerity::ModifySingletonQuery modifyPlatformerLoading;

    Emergence::Celerity::InsertShortTermQuery insertConfigRequest;
    Emergence::Celerity::FetchSequenceQuery fetchConfigResponse;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertCameraContext;
    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;
    Emergence::Celerity::InsertLongTermQuery insertUIPass;

    Emergence::Celerity::InsertLongTermQuery insertKeyTrigger;
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
      modifyPlatformerLoading (MODIFY_SINGLETON (PlatformerLoadingSingleton)),

      insertConfigRequest (INSERT_SHORT_TERM (Emergence::Celerity::ResourceConfigRequest)),
      fetchConfigResponse (FETCH_SEQUENCE (Emergence::Celerity::ResourceConfigLoadedResponse)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertCameraContext (INSERT_LONG_TERM (CameraContextComponent)),
      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),
      insertUIPass (INSERT_LONG_TERM (Emergence::Celerity::UIRenderPass)),

      insertKeyTrigger (INSERT_LONG_TERM (Emergence::Celerity::KeyTrigger))
{
    _constructor.DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (LevelLoading::Checkpoint::STARTED);
    _constructor.MakeDependencyOf (LoadingAnimation::Checkpoint::STARTED);
}

void LoadingOrchestrator::Execute () noexcept
{
    auto gameStateCursor = modifyGameState.Execute();
    auto *gameState = static_cast<GameStateSingleton *>(*gameStateCursor);

    if (gameState->state != GameState::PLATFORMER_LOADING)
    {
        // Waiting for transition to happen.
        return;
    }

    auto platformerLoadingCursor = modifyPlatformerLoading.Execute ();
    auto *platformerLoading = static_cast<PlatformerLoadingSingleton *> (*platformerLoadingCursor);

    if (platformerLoading->loadingStartTimeNs == 0u)
    {
        platformerLoading->loadingStartTimeNs = Emergence::Time::NanosecondsSinceStartup ();
    }

    if (!platformerLoading->characterAnimationConfigurationsLoadingRequested)
    {
        auto requestCursor = insertConfigRequest.Execute ();
        auto *request = static_cast<Emergence::Celerity::ResourceConfigRequest *> (++requestCursor);
        request->type = CharacterAnimationConfiguration::Reflect ().mapping;
        platformerLoading->characterAnimationConfigurationsLoadingRequested = true;
    }

    if (!platformerLoading->dynamicsMaterialsLoadingRequested)
    {
        auto requestCursor = insertConfigRequest.Execute ();
        auto *request = static_cast<Emergence::Celerity::ResourceConfigRequest *> (++requestCursor);
        request->type = Emergence::Celerity::DynamicsMaterial2d::Reflect ().mapping;
        platformerLoading->dynamicsMaterialsLoadingRequested = true;
    }

    if (!platformerLoading->movementConfigurationsLoadingRequested)
    {
        auto requestCursor = insertConfigRequest.Execute ();
        auto *request = static_cast<Emergence::Celerity::ResourceConfigRequest *> (++requestCursor);
        request->type = MovementConfiguration::Reflect ().mapping;
        platformerLoading->movementConfigurationsLoadingRequested = true;
    }

    for (auto responseCursor = fetchConfigResponse.Execute ();
         const auto *response =
             static_cast<const Emergence::Celerity::ResourceConfigLoadedResponse *> (*responseCursor);
         ++responseCursor)
    {
        if (response->type == CharacterAnimationConfiguration::Reflect ().mapping)
        {
            platformerLoading->characterAnimationConfigurationsLoaded = true;
        }
        else if (response->type == Emergence::Celerity::DynamicsMaterial2d::Reflect ().mapping)
        {
            platformerLoading->dynamicsMaterialsLoaded = true;
        }
        else if (response->type == MovementConfiguration::Reflect ().mapping)
        {
            platformerLoading->movementConfigurationsLoaded = true;
        }
    }

    auto levelLoadingCursor = modifyLevelLoading.Execute ();
    auto *levelLoading = static_cast<LevelLoadingSingleton *> (*levelLoadingCursor);

    // Load level only after all configs are loaded.
    if (platformerLoading->dynamicsMaterialsLoaded)
    {
        if (!*levelLoading->levelName && *gameState->lastRequest.levelName)
        {
            levelLoading->levelName = gameState->lastRequest.levelName;
        }
    }

    auto localeCursor = modifyLocale.Execute ();
    auto *locale = static_cast<Emergence::Celerity::LocaleSingleton *> (*localeCursor);

    if (!*locale->targetLocale)
    {
        locale->targetLocale = Localization::HARDCODED_LOCALE;
    }

    auto levelsConfigurationCursor = fetchLevelsConfiguration.Execute ();
    const auto *levelsConfiguration = static_cast<const LevelsConfigurationSingleton *> (*levelsConfigurationCursor);

    auto assetManagerCursor = fetchAssetManager.Execute ();
    const auto *assetManager = static_cast<const Emergence::Celerity::AssetManagerSingleton *> (*assetManagerCursor);
    const bool assetsWereLoaded = platformerLoading->assetsLoaded;
    platformerLoading->assetsLoaded = assetManager->assetsLeftToLoad == 0u;

    auto loadingAnimationCursor = modifyLoadingAnimation.Execute ();
    auto *loadingAnimation = static_cast<LoadingAnimationSingleton *> (*loadingAnimationCursor);

    if (levelsConfiguration->loaded && levelLoading->state == LevelLoadingState::DONE && assetsWereLoaded &&
        platformerLoading->assetsLoaded && platformerLoading->characterAnimationConfigurationsLoaded &&
        platformerLoading->dynamicsMaterialsLoaded && locale->loadedLocale == locale->targetLocale)
    {
        loadingAnimation->required = false;
        SpawnViewports ();
        InitInput ();
        gameState->request.state = GameState::PLATFORMER_GAME;

        const std::uint64_t loadingTimeNs =
            Emergence::Time::NanosecondsSinceStartup () - platformerLoading->loadingStartTimeNs;
        platformerLoading->loadingStartTimeNs = 0u;

        EMERGENCE_LOG (INFO, "PlatformerLoadingOrchestration: Scene loading took ",
                       static_cast<float> (loadingTimeNs) * 1e-9f, " seconds.");
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

    auto cameraContextCursor = insertCameraContext.Execute ();
    auto *cameraContext = static_cast<CameraContextComponent *> (++cameraContextCursor);
    cameraContext->objectId = cameraTransform->GetObjectId ();
    cameraContext->target = CameraTarget::PLAYER;

    auto viewportCursor = insertViewport.Execute ();
    auto *worldViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);

    worldViewport->name = "PlatformerScene"_us;
    worldViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
    worldViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
    worldViewport->clearColor = 0xC3FBFFFF;
    worldViewport->sortIndex = 0u;

    auto worldPassCursor = insertWorldPass.Execute ();
    auto *worldPass = static_cast<Emergence::Celerity::World2dRenderPass *> (++worldPassCursor);
    worldPass->name = worldViewport->name;
    worldPass->cameraObjectId = camera->objectId;

    auto *uiViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);
    uiViewport->name = "PlatformerUI"_us;
    uiViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
    uiViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
    uiViewport->clearColor = 0x00000000;
    uiViewport->sortIndex = 1u;

    auto uiPassCursor = insertUIPass.Execute ();
    auto *uiPass = static_cast<Emergence::Celerity::UIRenderPass *> (++uiPassCursor);
    uiPass->name = uiViewport->name;
    uiPass->defaultStyleId = "Default"_us;
}

void LoadingOrchestrator::InitInput () noexcept
{
    auto keyTriggerCursor = insertKeyTrigger.Execute ();
    auto *upTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    upTrigger->actionToSend = PlatformerInputActions::DIRECTED_MOVEMENT;
    upTrigger->actionToSend.real[0u] = 0.0f;
    upTrigger->actionToSend.real[1u] = 1.0f;
    // TODO: Normally, we need to get this codes from settings, not just hardcode SDL values.
    upTrigger->triggerCode = 26;
    upTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *downTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    downTrigger->actionToSend = PlatformerInputActions::DIRECTED_MOVEMENT;
    downTrigger->actionToSend.real[0u] = 0.0f;
    downTrigger->actionToSend.real[1u] = -1.0f;
    downTrigger->triggerCode = 22;
    downTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *leftTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    leftTrigger->actionToSend = PlatformerInputActions::DIRECTED_MOVEMENT;
    leftTrigger->actionToSend.real[0u] = -1.0f;
    leftTrigger->actionToSend.real[1u] = 0.0f;
    leftTrigger->triggerCode = 4;
    leftTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *rightTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    rightTrigger->actionToSend = PlatformerInputActions::DIRECTED_MOVEMENT;
    rightTrigger->actionToSend.real[0u] = 1.0f;
    rightTrigger->actionToSend.real[1u] = 0.0f;
    rightTrigger->triggerCode = 7;
    rightTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *rollTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    rollTrigger->actionToSend = PlatformerInputActions::MOVEMENT_ROLL;
    rollTrigger->triggerCode = 44;
    rollTrigger->triggerType = Emergence::Celerity::KeyTriggerType::ON_STATE_CHANGED;
    rollTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"PlatformerLoadingOrchestrator"})
        .SetExecutor<LoadingOrchestrator> ();
}
} // namespace PlatformerLoadingOrchestration
