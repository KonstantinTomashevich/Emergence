#include <Celerity/Assembly/PrototypeAssemblyComponent.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/Physics3d/DynamicsMaterial3d.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Resource/Config/Loading.hpp>
#include <Celerity/Resource/Config/Messages.hpp>
#include <Celerity/Resource/Object/Loading.hpp>
#include <Celerity/Resource/Object/Messages.hpp>

#include <Container/Vector.hpp>

#include <Loading/Model/Events.hpp>
#include <Loading/Model/Messages.hpp>
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
    WAITING_FOR_ASSEMBLY,
    FINISHED,
};

class LoadingOrchestrator final : public Emergence::Celerity::TaskExecutorBase<LoadingOrchestrator>
{
public:
    LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor, bool *_loadingFinishedOutput) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyWorld;

    Emergence::Celerity::FetchSequenceQuery fetchResourceConfigResponse;
    Emergence::Celerity::FetchSequenceQuery fetchResourceObjectResponse;
    Emergence::Celerity::FetchSequenceQuery fetchLevelGenerationResponse;

    Emergence::Celerity::InsertShortTermQuery insertResourceConfigRequest;
    Emergence::Celerity::InsertShortTermQuery insertResourceObjectRequest;
    Emergence::Celerity::InsertShortTermQuery insertLevelGenerationRequest;
    Emergence::Celerity::InsertShortTermQuery insertLoadingFinishedEvent;

    Emergence::Celerity::FetchSignalQuery fetchUninitializedPrototypes;
    Emergence::Celerity::FetchAscendingRangeQuery fetchPrototypeStates;

    LoadingStage stage = LoadingStage::NOT_STARTED;
    bool resourceObjectsLoaded = false;
    bool dynamicsMaterialsLoaded = false;

    Emergence::Container::Vector<Emergence::Memory::UniqueString> objectsLeftToBeLoaded {
        Emergence::Memory::Profiler::AllocationGroup::Top ()};

    bool *loadingFinishedOutput = nullptr;
};

LoadingOrchestrator::LoadingOrchestrator (Emergence::Celerity::TaskConstructor &_constructor,
                                          bool *_loadingFinishedOutput) noexcept
    : modifyWorld (MODIFY_SINGLETON (Emergence::Celerity::WorldSingleton)),

      fetchResourceConfigResponse (FETCH_SEQUENCE (Emergence::Celerity::ResourceConfigLoadedResponse)),
      fetchResourceObjectResponse (FETCH_SEQUENCE (Emergence::Celerity::ResourceObjectLoadedResponse)),
      fetchLevelGenerationResponse (FETCH_SEQUENCE (LevelGenerationFinishedResponse)),

      insertResourceConfigRequest (INSERT_SHORT_TERM (Emergence::Celerity::ResourceConfigRequest)),
      insertResourceObjectRequest (INSERT_SHORT_TERM (Emergence::Celerity::ResourceObjectRequest)),
      insertLevelGenerationRequest (INSERT_SHORT_TERM (LevelGenerationRequest)),
      insertLoadingFinishedEvent (INSERT_SHORT_TERM (LoadingFinishedEvent)),

      fetchUninitializedPrototypes (FETCH_SIGNAL (Emergence::Celerity::PrototypeComponent, assemblyStarted, false)),
      fetchPrototypeStates (FETCH_ASCENDING_RANGE (Emergence::Celerity::PrototypeAssemblyComponent, objectId)),

      loadingFinishedOutput (_loadingFinishedOutput)
{
    _constructor.DependOn (Emergence::Celerity::ResourceConfigLoading::Checkpoint::FINISHED);
    _constructor.DependOn (Emergence::Celerity::ResourceObjectLoading::Checkpoint::FINISHED);
    _constructor.DependOn (LevelGeneration::Checkpoint::FINISHED);
    _constructor.DependOn (PhysicsInitialization::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    *loadingFinishedOutput = false;

    objectsLeftToBeLoaded.emplace_back ("RO_FloorTile");
    objectsLeftToBeLoaded.emplace_back ("RO_ObstacleRed");
    objectsLeftToBeLoaded.emplace_back ("RO_ObstacleYellow");
    objectsLeftToBeLoaded.emplace_back ("RO_Fighter");
}

void LoadingOrchestrator::Execute () noexcept
{
    switch (stage)
    {
    case LoadingStage::NOT_STARTED:
    {
        auto resourceObjectRequestCursor = insertResourceObjectRequest.Execute ();
        for (Emergence::Memory::UniqueString objectId : objectsLeftToBeLoaded)
        {
            auto *resourceObjectRequest =
                static_cast<Emergence::Celerity::ResourceObjectRequest *> (++resourceObjectRequestCursor);
            resourceObjectRequest->objectId = objectId;
        }

        auto resourceConfigRequestCursor = insertResourceConfigRequest.Execute ();
        auto *dynamicsMaterialRequest =
            static_cast<Emergence::Celerity::ResourceConfigRequest *> (++resourceConfigRequestCursor);
        dynamicsMaterialRequest->type = Emergence::Celerity::DynamicsMaterial3d::Reflect ().mapping;

        stage = LoadingStage::LOADING_LEVEL_ASSETS;
        break;
    }

    case LoadingStage::LOADING_LEVEL_ASSETS:
    {
        if (!resourceObjectsLoaded)
        {
            for (auto cursor = fetchResourceObjectResponse.Execute ();
                 const auto *response =
                     static_cast<const Emergence::Celerity::ResourceObjectLoadedResponse *> (*cursor);
                 ++cursor)
            {
                if (auto iterator =
                        std::find (objectsLeftToBeLoaded.begin (), objectsLeftToBeLoaded.end (), response->objectId);
                    iterator != objectsLeftToBeLoaded.end ())
                {
                    objectsLeftToBeLoaded.erase (iterator);
                    resourceObjectsLoaded = objectsLeftToBeLoaded.empty ();
                }
            }
        }

        if (!dynamicsMaterialsLoaded)
        {
            for (auto cursor = fetchResourceConfigResponse.Execute ();
                 const auto *response =
                     static_cast<const Emergence::Celerity::ResourceConfigLoadedResponse *> (*cursor);
                 ++cursor)
            {
                if (response->type == Emergence::Celerity::DynamicsMaterial3d::Reflect ().mapping)
                {
                    dynamicsMaterialsLoaded = true;
                    break;
                }
            }
        }

        if (resourceObjectsLoaded && dynamicsMaterialsLoaded)
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
            stage = LoadingStage::WAITING_FOR_ASSEMBLY;
        }

        break;
    }

    case LoadingStage::WAITING_FOR_ASSEMBLY:
    {
        if (!*fetchUninitializedPrototypes.Execute () && !*fetchPrototypeStates.Execute (nullptr, nullptr))
        {
            stage = LoadingStage::FINISHED;
        }

        break;
    }

    case LoadingStage::FINISHED:
        *loadingFinishedOutput = true;
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
    EMERGENCE_ASSERT (_loadingFinishedOutput);
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("LoadingOrchestration");
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"LoadingOrchestrator"})
        .SetExecutor<LoadingOrchestrator> (_loadingFinishedOutput);
}
} // namespace LoadingOrchestration
