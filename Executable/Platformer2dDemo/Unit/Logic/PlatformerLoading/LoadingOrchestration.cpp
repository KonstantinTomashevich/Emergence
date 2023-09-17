#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Locale/LocaleSingleton.hpp>
#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Resource/Config/Messages.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <Configuration/Localization.hpp>

#include <GameCore/GameStateSingleton.hpp>
#include <GameCore/LevelsConfigurationSingleton.hpp>

#include <LevelLoading/LevelLoading.hpp>
#include <LevelLoading/LevelLoadingSingleton.hpp>

#include <LoadingAnimation/LoadingAnimation.hpp>
#include <LoadingAnimation/LoadingAnimationSingleton.hpp>

#include <Log/Log.hpp>

#include <Platformer/Animation/CharacterAnimationConfiguration.hpp>
#include <Platformer/Movement/MovementConfiguration.hpp>

#include <PlatformerLoading/LoadingOrchestration.hpp>
#include <PlatformerLoading/PlatformerLoadingSingleton.hpp>

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
    Emergence::Celerity::FetchSingletonQuery fetchAssetManager;
    Emergence::Celerity::FetchSingletonQuery fetchLevelsConfiguration;
    Emergence::Celerity::ModifySingletonQuery modifyGameState;
    Emergence::Celerity::ModifySingletonQuery modifyLevelLoading;
    Emergence::Celerity::ModifySingletonQuery modifyLoadingAnimation;
    Emergence::Celerity::ModifySingletonQuery modifyLocale;
    Emergence::Celerity::ModifySingletonQuery modifyPlatformerLoading;

    Emergence::Celerity::InsertShortTermQuery insertConfigRequest;
    Emergence::Celerity::FetchSequenceQuery fetchConfigResponse;
};

LoadingOrchestrator::LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      fetchAssetManager (FETCH_SINGLETON (Emergence::Celerity::AssetManagerSingleton)),
      fetchLevelsConfiguration (FETCH_SINGLETON (LevelsConfigurationSingleton)),
      modifyGameState (MODIFY_SINGLETON (GameStateSingleton)),
      modifyLevelLoading (MODIFY_SINGLETON (LevelLoadingSingleton)),
      modifyLoadingAnimation (MODIFY_SINGLETON (LoadingAnimationSingleton)),
      modifyLocale (MODIFY_SINGLETON (Emergence::Celerity::LocaleSingleton)),
      modifyPlatformerLoading (MODIFY_SINGLETON (PlatformerLoadingSingleton)),

      insertConfigRequest (INSERT_SHORT_TERM (Emergence::Celerity::ResourceConfigRequest)),
      fetchConfigResponse (FETCH_SEQUENCE (Emergence::Celerity::ResourceConfigLoadedResponse))
{
    _constructor.DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);
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

    auto gameStateCursor = modifyGameState.Execute ();
    auto *gameState = static_cast<GameStateSingleton *> (*gameStateCursor);

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
        if (gameState->lastRequest.state != GameState::PLATFORMER_GAME)
        {
            gameState->request.state = GameState::PLATFORMER_GAME;

            const std::uint64_t loadingTimeNs =
                Emergence::Time::NanosecondsSinceStartup () - platformerLoading->loadingStartTimeNs;
            platformerLoading->loadingStartTimeNs = 0u;

            EMERGENCE_LOG (INFO, "PlatformerLoadingOrchestration: Scene loading took ",
                           static_cast<float> (loadingTimeNs) * 1e-9f, " seconds.");
        }
    }
    else
    {
        loadingAnimation->required = true;
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"PlatformerLoadingOrchestrator"})
        .SetExecutor<LoadingOrchestrator> ();
}
} // namespace PlatformerLoadingOrchestration
