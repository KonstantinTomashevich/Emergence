#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Input/InputTriggers.hpp>
#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Resource/Config/Messages.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/UI/UIRenderPass.hpp>

#include <Configuration/Localization.hpp>
#include <Configuration/VisibilityMask.hpp>

#include <Framework/GameState.hpp>

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

#include <Root/LevelSelectionSingleton.hpp>
#include <Root/LevelsConfigurationSingleton.hpp>

#include <SyntaxSugar/Time.hpp>

namespace PlatformerLoadingOrchestration
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"PlatformerLoadingOrchestration::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"PlatformerLoadingOrchestration::Finished"};

class LoadingOrchestrator final : public Emergence::Celerity::TaskExecutorBase<LoadingOrchestrator>
{
public:
    LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor,
                         const ViewDropHandle &_viewDropHandle,
                         Emergence::Celerity::WorldView *_ownerView) noexcept;

    void Execute () noexcept;

private:
    void SpawnViewports (const Emergence::Celerity::WorldSingleton *_world) noexcept;

    void InitInput () noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchAssetManager;
    Emergence::Celerity::FetchSingletonQuery fetchLevelsConfiguration;
    Emergence::Celerity::FetchSingletonQuery fetchLevelSelection;
    Emergence::Celerity::ModifySingletonQuery modifyLevelLoading;
    Emergence::Celerity::ModifySingletonQuery modifyLoadingAnimation;
    Emergence::Celerity::ModifySingletonQuery modifyLocale;
    Emergence::Celerity::ModifySingletonQuery modifyPlatformerLoading;
    Emergence::Celerity::ModifySingletonQuery modifyWorld;

    Emergence::Celerity::InsertShortTermQuery insertConfigRequest;
    Emergence::Celerity::FetchSequenceQuery fetchConfigResponse;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertCameraContext;
    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;
    Emergence::Celerity::InsertLongTermQuery insertUIPass;

    Emergence::Celerity::InsertLongTermQuery insertKeyTrigger;

    ViewDropHandle viewDropHandle;
    Emergence::Celerity::WorldView *ownerView;
};

LoadingOrchestrator::LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor,
                                          const ViewDropHandle &_viewDropHandle,
                                          Emergence::Celerity::WorldView *_ownerView) noexcept
    : fetchAssetManager (FETCH_SINGLETON (Emergence::Celerity::AssetManagerSingleton)),
      fetchLevelsConfiguration (FETCH_SINGLETON (LevelsConfigurationSingleton)),
      fetchLevelSelection (FETCH_SINGLETON (LevelSelectionSingleton)),
      modifyLevelLoading (MODIFY_SINGLETON (LevelLoadingSingleton)),
      modifyLoadingAnimation (MODIFY_SINGLETON (LoadingAnimationSingleton)),
      modifyLocale (MODIFY_SINGLETON (Emergence::Celerity::LocaleSingleton)),
      modifyPlatformerLoading (MODIFY_SINGLETON (PlatformerLoadingSingleton)),
      modifyWorld (MODIFY_SINGLETON (Emergence::Celerity::WorldSingleton)),

      insertConfigRequest (INSERT_SHORT_TERM (Emergence::Celerity::ResourceConfigRequest)),
      fetchConfigResponse (FETCH_SEQUENCE (Emergence::Celerity::ResourceConfigLoadedResponse)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertCameraContext (INSERT_LONG_TERM (CameraContextComponent)),
      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),
      insertUIPass (INSERT_LONG_TERM (Emergence::Celerity::UIRenderPass)),

      insertKeyTrigger (INSERT_LONG_TERM (Emergence::Celerity::KeyTrigger)),

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

    auto worldCursor = modifyWorld.Execute ();
    auto *world = static_cast<Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto levelSelectionCursor = fetchLevelSelection.Execute ();
    const auto *levelSelection = static_cast<const LevelSelectionSingleton *> (*levelSelectionCursor);

    auto levelLoadingCursor = modifyLevelLoading.Execute ();
    auto *levelLoading = static_cast<LevelLoadingSingleton *> (*levelLoadingCursor);

    // Load level only after all configs are loaded.
    if (platformerLoading->dynamicsMaterialsLoaded)
    {
        if (!*levelLoading->levelName && *levelSelection->selectedLevelName)
        {
            levelLoading->levelName = levelSelection->selectedLevelName;
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
    platformerLoading->assetsLoaded = assetManager->assetsLeftToLoad == 0u;

    auto loadingAnimationCursor = modifyLoadingAnimation.Execute ();
    auto *loadingAnimation = static_cast<LoadingAnimationSingleton *> (*loadingAnimationCursor);

    if (levelsConfiguration->loaded && levelLoading->state == LevelLoadingState::DONE &&
        platformerLoading->assetsLoaded && platformerLoading->characterAnimationConfigurationsLoaded &&
        platformerLoading->dynamicsMaterialsLoaded && locale->loadedLocale == locale->targetLocale)
    {
        loadingAnimation->required = false;
        world->updateMode = Emergence::Celerity::WorldUpdateMode::SIMULATING;
        SpawnViewports (world);
        InitInput ();
        viewDropHandle.RequestViewDrop (ownerView);

        const uint64_t loadingTimeNs =
            Emergence::Time::NanosecondsSinceStartup () - platformerLoading->loadingStartTimeNs;
        platformerLoading->loadingStartTimeNs = 0u;

        EMERGENCE_LOG (INFO, "PlatformerLoadingOrchestration: Scene loading took ",
                       static_cast<float> (loadingTimeNs) * 1e-9f, " seconds.");
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

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        const ViewDropHandle &_viewDropHandle,
                        Emergence::Celerity::WorldView *_ownerView) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"PlatformerLoadingOrchestrator"})
        .SetExecutor<LoadingOrchestrator> (_viewDropHandle, _ownerView);
}
} // namespace PlatformerLoadingOrchestration
