#include <cassert>

#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Loading/Model/AssemblyDescriptorLoadingSingleton.hpp>
#include <Loading/Model/DynamicsMaterialLoadingSingleton.hpp>
#include <Loading/Model/InputInitializationSingleton.hpp>
#include <Loading/Model/LevelGenerationSingleton.hpp>
#include <Loading/Model/PhysicsInitializationSingleton.hpp>
#include <Loading/Task/AssemblyDescriptorLoading.hpp>
#include <Loading/Task/DynamicsMaterialLoading.hpp>
#include <Loading/Task/InputInitialization.hpp>
#include <Loading/Task/LevelGeneration.hpp>
#include <Loading/Task/LoadingStatusCheck.hpp>
#include <Loading/Task/PhysicsInitialization.hpp>

namespace LoadingStatusCheck
{
class LoadingStatusChecker final : public Emergence::Celerity::TaskExecutorBase<LoadingStatusChecker>
{
public:
    LoadingStatusChecker (Emergence::Celerity::TaskConstructor &_constructor, bool *_loadingFinishedOutput) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchAssemblyDescriptorLoadingState;
    Emergence::Celerity::FetchSingletonQuery fetchDynamicsMaterialLoadingState;
    Emergence::Celerity::FetchSingletonQuery fetchInputInitializationState;
    Emergence::Celerity::FetchSingletonQuery fetchLevelGenerationState;
    Emergence::Celerity::FetchSingletonQuery fetchPhysicsInitializationState;
    Emergence::Celerity::ModifySingletonQuery modifyWorld;

    bool *loadingFinishedOutput = nullptr;
};

LoadingStatusChecker::LoadingStatusChecker (Emergence::Celerity::TaskConstructor &_constructor,
                                            bool *_loadingFinishedOutput) noexcept
    : fetchAssemblyDescriptorLoadingState (FETCH_SINGLETON (AssemblyDescriptorLoadingSingleton)),
      fetchDynamicsMaterialLoadingState (FETCH_SINGLETON (DynamicsMaterialLoadingSingleton)),
      fetchInputInitializationState (FETCH_SINGLETON (InputInitializationSingleton)),
      fetchLevelGenerationState (FETCH_SINGLETON (LevelGenerationSingleton)),
      fetchPhysicsInitializationState (FETCH_SINGLETON (PhysicsInitializationSingleton)),
      modifyWorld (MODIFY_SINGLETON (Emergence::Celerity::WorldSingleton)),
      loadingFinishedOutput (_loadingFinishedOutput)
{
    _constructor.DependOn (AssemblyDescriptorLoading::Checkpoint::STEP_FINISHED);
    _constructor.DependOn (DynamicsMaterialLoading::Checkpoint::STEP_FINISHED);
    _constructor.DependOn (InputInitialization::Checkpoint::INITIALIZED);
    _constructor.DependOn (LevelGeneration::Checkpoint::STEP_FINISHED);
    _constructor.DependOn (PhysicsInitialization::Checkpoint::INITIALIZED);
}

void LoadingStatusChecker::Execute () noexcept
{
    auto assemblyDescriptorLoadingStateCursor = fetchAssemblyDescriptorLoadingState.Execute ();
    const auto *assemblyDescriptorLoadingState =
        static_cast<const AssemblyDescriptorLoadingSingleton *> (*assemblyDescriptorLoadingStateCursor);

    auto dynamicsMaterialLoadingStateCursor = fetchDynamicsMaterialLoadingState.Execute ();
    const auto *dynamicsMaterialLoadingState =
        static_cast<const DynamicsMaterialLoadingSingleton *> (*dynamicsMaterialLoadingStateCursor);

    auto inputInitializationStateCursor = fetchInputInitializationState.Execute ();
    const auto *inputInitializationState =
        static_cast<const InputInitializationSingleton *> (*inputInitializationStateCursor);

    auto levelGenerationStateCursor = fetchLevelGenerationState.Execute ();
    const auto *levelGenerationState = static_cast<const LevelGenerationSingleton *> (*levelGenerationStateCursor);

    auto physicsInitializationStateCursor = fetchPhysicsInitializationState.Execute ();
    const auto *physicsInitializationState =
        static_cast<const PhysicsInitializationSingleton *> (*physicsInitializationStateCursor);

    const bool loadingFinished = assemblyDescriptorLoadingState->finished && dynamicsMaterialLoadingState->finished &&
                                 inputInitializationState->finished && levelGenerationState->finished &&
                                 physicsInitializationState->finished;

    auto worldCursor = modifyWorld.Execute ();
    auto *world = static_cast<Emergence::Celerity::WorldSingleton *> (*worldCursor);

    world->updateMode = loadingFinished ? Emergence::Celerity::WorldUpdateMode::SIMULATING :
                                          Emergence::Celerity::WorldUpdateMode::FROZEN;
    *loadingFinishedOutput = loadingFinished;
}

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                                  bool *_loadingFinishedOutput) noexcept
{
    assert (_loadingFinishedOutput);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"LoadingStatusChecker"})
        .SetExecutor<LoadingStatusChecker> (_loadingFinishedOutput);
}
} // namespace LoadingStatusCheck
