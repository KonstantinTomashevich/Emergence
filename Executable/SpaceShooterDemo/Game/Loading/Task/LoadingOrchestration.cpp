#include <cassert>

#include <Celerity/Asset/Object/Loading.hpp>
#include <Celerity/Asset/Object/Messages.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Loading/Model/Events.hpp>
#include <Loading/Model/Messages.hpp>
#include <Loading/Task/DynamicsMaterialLoading.hpp>
#include <Loading/Task/LevelGeneration.hpp>
#include <Loading/Task/LoadingOrchestration.hpp>
#include <Loading/Task/PhysicsInitialization.hpp>

namespace LoadingOrchestration
{
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"LoadingOrchestrationFinished"};

enum class LoadingStage
{
    NOT_STARTED = 0u,
    LOADING_LEVEL_ASSETS,
    GENERATING_LEVEL,
    FINISHED,
};

class LoadingOrchestrator final : public Emergence::Celerity::TaskExecutorBase<LoadingOrchestrator>
{
public:
    LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor, bool *_loadingFinishedOutput) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyWorld;

    Emergence::Celerity::FetchSequenceQuery fetchAssetObjectFolderResponse;
    Emergence::Celerity::FetchSequenceQuery fetchDynamicsMaterialResponse;
    Emergence::Celerity::FetchSequenceQuery fetchLevelGenerationResponse;

    Emergence::Celerity::InsertShortTermQuery insertAssetObjectFolderRequest;
    Emergence::Celerity::InsertShortTermQuery insertDynamicsMaterialRequest;
    Emergence::Celerity::InsertShortTermQuery insertLevelGenerationRequest;
    Emergence::Celerity::InsertShortTermQuery insertLoadingFinishedEvent;

    LoadingStage stage = LoadingStage::NOT_STARTED;
    bool assetObjectsLoaded = false;
    bool dynamicsMaterialsLoaded = false;

    bool *loadingFinishedOutput = nullptr;
};

LoadingOrchestrator::LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor,
                                          bool *_loadingFinishedOutput) noexcept
    : modifyWorld (MODIFY_SINGLETON (Emergence::Celerity::WorldSingleton)),

      fetchAssetObjectFolderResponse (FETCH_SEQUENCE (Emergence::Celerity::AssetObjectFolderLoadedResponse)),
      fetchDynamicsMaterialResponse (FETCH_SEQUENCE (DynamicsMaterialLoadingFinishedResponse)),
      fetchLevelGenerationResponse (FETCH_SEQUENCE (LevelGenerationFinishedResponse)),

      insertAssetObjectFolderRequest (INSERT_SHORT_TERM (Emergence::Celerity::AssetObjectFolderRequest)),
      insertDynamicsMaterialRequest (INSERT_SHORT_TERM (DynamicsMaterialLoadingRequest)),
      insertLevelGenerationRequest (INSERT_SHORT_TERM (LevelGenerationRequest)),
      insertLoadingFinishedEvent (INSERT_SHORT_TERM (LoadingFinishedEvent)),

      loadingFinishedOutput (_loadingFinishedOutput)
{
    _constructor.DependOn (Emergence::Celerity::AssetObjectLoading::Checkpoint::FINISHED);
    _constructor.DependOn (DynamicsMaterialLoading::Checkpoint::FINISHED);
    _constructor.DependOn (LevelGeneration::Checkpoint::FINISHED);
    _constructor.DependOn (PhysicsInitialization::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    *loadingFinishedOutput = false;
}

void LoadingOrchestrator::Execute () noexcept
{
    static const char *const assetObjectFolder = "../GameAssets/Objects/";
    static const char *const dynamicsMaterialFolder = "../GameAssets/Physics/Materials/";

    switch (stage)
    {
    case LoadingStage::NOT_STARTED:
    {
        auto assetObjectRequestCursor = insertAssetObjectFolderRequest.Execute ();
        auto *assetObjectRequest =
            static_cast<Emergence::Celerity::AssetObjectFolderRequest *> (++assetObjectRequestCursor);
        assetObjectRequest->folder = assetObjectFolder;

        auto dynamicsMaterialRequestCursor = insertDynamicsMaterialRequest.Execute ();
        auto *dynamicsMaterialRequest = static_cast<DynamicsMaterialLoadingRequest *> (++dynamicsMaterialRequestCursor);
        dynamicsMaterialRequest->folder = dynamicsMaterialFolder;

        stage = LoadingStage::LOADING_LEVEL_ASSETS;
        break;
    }

    case LoadingStage::LOADING_LEVEL_ASSETS:
    {
        for (auto cursor = fetchAssetObjectFolderResponse.Execute ();
             const auto *response = static_cast<const Emergence::Celerity::AssetObjectFolderLoadedResponse *> (*cursor);
             ++cursor)
        {
            if (response->folder == assetObjectFolder)
            {
                assetObjectsLoaded = true;
                break;
            }
        }

        for (auto cursor = fetchDynamicsMaterialResponse.Execute ();
             const auto *response = static_cast<const DynamicsMaterialLoadingFinishedResponse *> (*cursor); ++cursor)
        {
            if (response->folder == dynamicsMaterialFolder)
            {
                dynamicsMaterialsLoaded = true;
                break;
            }
        }

        if (assetObjectsLoaded && dynamicsMaterialsLoaded)
        {
            auto levelGenerationRequestCursor = insertLevelGenerationRequest.Execute ();
            ++levelGenerationRequestCursor;
            stage = LoadingStage::GENERATING_LEVEL;
        }

        break;
    }

    case LoadingStage::GENERATING_LEVEL:
    {
        auto levelGenerationResponseCursor = fetchLevelGenerationResponse.Execute ();
        if (*levelGenerationResponseCursor)
        {
            auto loadingFinishedCursor = insertLoadingFinishedEvent.Execute ();
            ++loadingFinishedCursor;

            *loadingFinishedOutput = true;
            stage = LoadingStage::FINISHED;
        }

        break;
    }

    case LoadingStage::FINISHED:
        break;
    }

    auto worldCursor = modifyWorld.Execute ();
    auto *world = static_cast<Emergence::Celerity::WorldSingleton *> (*worldCursor);

    world->updateMode = stage == LoadingStage::FINISHED ? Emergence::Celerity::WorldUpdateMode::SIMULATING :
                                                          Emergence::Celerity::WorldUpdateMode::FROZEN;
}

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                           bool *_loadingFinishedOutput) noexcept
{
    assert (_loadingFinishedOutput);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"LoadingOrchestrator"})
        .SetExecutor<LoadingOrchestrator> (_loadingFinishedOutput);
}
} // namespace LoadingOrchestration
