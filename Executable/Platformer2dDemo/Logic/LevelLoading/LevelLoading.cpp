#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/Assembly/PrototypeAssemblyComponent.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Resource/Object/Messages.hpp>
#include <Celerity/Transform/TransformComponent.hpp>

#include <LevelLoading/LevelLoading.hpp>
#include <LevelLoading/LevelLoadingSingleton.hpp>

#include <LoadingAnimation/LoadingAnimation.hpp>

#include <Log/Log.hpp>

namespace LevelLoading
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"LevelLoading::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"LevelLoading::Finished"};

class LevelLoader final : public Emergence::Celerity::TaskExecutorBase<LevelLoader>
{
public:
    LevelLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void RequestDescriptor (const LevelLoadingSingleton *_levelLoading) noexcept;

    bool CheckDescriptorLoadingResponse (const LevelLoadingSingleton *_levelLoading) noexcept;

    void SpawnLevelPrototype (const LevelLoadingSingleton *_levelLoading) noexcept;

    bool IsAnyAssemblyGoingOn () noexcept;

    // TODO: Only level descriptor is getting cleaned up, but not its sub objects.
    //       It should be ok, as sub objects are usually small, while levels are large and chunky.
    void CleanupDescriptor (const LevelLoadingSingleton *_levelLoading) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::ModifySingletonQuery modifyLevelLoading;

    Emergence::Celerity::InsertShortTermQuery insertResourceObjectRequest;
    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertPrototype;

    Emergence::Celerity::FetchSequenceQuery fetchResourceObjectLoadedResponses;
    Emergence::Celerity::FetchAscendingRangeQuery fetchPrototypeAssemblyByAscendingId;
    Emergence::Celerity::RemoveValueQuery removeAssemblyDescriptorById;
};

LevelLoader::LevelLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      modifyLevelLoading (MODIFY_SINGLETON (LevelLoadingSingleton)),

      insertResourceObjectRequest (INSERT_SHORT_TERM (Emergence::Celerity::ResourceObjectRequest)),
      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertPrototype (INSERT_LONG_TERM (Emergence::Celerity::PrototypeComponent)),

      fetchResourceObjectLoadedResponses (FETCH_SEQUENCE (Emergence::Celerity::ResourceObjectLoadedResponse)),
      fetchPrototypeAssemblyByAscendingId (
          FETCH_ASCENDING_RANGE (Emergence::Celerity::PrototypeAssemblyComponent, objectId)),
      removeAssemblyDescriptorById (REMOVE_VALUE_1F (Emergence::Celerity::AssemblyDescriptor, id))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (LoadingAnimation::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void LevelLoader::Execute () noexcept
{
    auto levelLoadingCursor = modifyLevelLoading.Execute ();
    auto *levelLoading = static_cast<LevelLoadingSingleton *> (*levelLoadingCursor);

    if (!*levelLoading->levelName)
    {
        return;
    }

    switch (levelLoading->state)
    {
    case LevelLoadingState::NONE:
        RequestDescriptor (levelLoading);
        levelLoading->state = LevelLoadingState::LOADING_DESCRIPTOR;
        break;

    case LevelLoadingState::LOADING_DESCRIPTOR:
        if (CheckDescriptorLoadingResponse (levelLoading))
        {
            SpawnLevelPrototype (levelLoading);
            levelLoading->state = LevelLoadingState::ASSEMBLING;
        }

        break;

    case LevelLoadingState::ASSEMBLING:
        if (!IsAnyAssemblyGoingOn ())
        {
            CleanupDescriptor (levelLoading);
            levelLoading->state = LevelLoadingState::DONE;
        }

        break;

    case LevelLoadingState::DONE:
        break;
    }
}

void LevelLoader::RequestDescriptor (const LevelLoadingSingleton *_levelLoading) noexcept
{
    auto cursor = insertResourceObjectRequest.Execute ();
    auto *request = static_cast<Emergence::Celerity::ResourceObjectRequest *> (++cursor);
    request->objects.emplace_back(_levelLoading->levelName);
}

bool LevelLoader::CheckDescriptorLoadingResponse (const LevelLoadingSingleton *_levelLoading) noexcept
{
    for (auto cursor = fetchResourceObjectLoadedResponses.Execute ();
         const auto *response = static_cast<const Emergence::Celerity::ResourceObjectLoadedResponse *> (*cursor);
         ++cursor)
    {
        if (response->objectId == _levelLoading->levelName)
        {
            return true;
        }
    }

    return false;
}

void LevelLoader::SpawnLevelPrototype (const LevelLoadingSingleton *_levelLoading) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto transformCursor = insertTransform.Execute ();
    auto *transform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
    transform->SetObjectId (world->GenerateId ());

    auto prototypeCursor = insertPrototype.Execute ();
    auto *prototype = static_cast<Emergence::Celerity::PrototypeComponent *> (++prototypeCursor);
    prototype->objectId = transform->GetObjectId ();
    prototype->descriptorId = _levelLoading->levelName;
    prototype->requestImmediateFixedAssembly = false;
    prototype->requestImmediateNormalAssembly = false;
}

bool LevelLoader::IsAnyAssemblyGoingOn () noexcept
{
    return *fetchPrototypeAssemblyByAscendingId.Execute (nullptr, nullptr);
}

void LevelLoader::CleanupDescriptor (const LevelLoadingSingleton *_levelLoading) noexcept
{
    auto cursor = removeAssemblyDescriptorById.Execute (&_levelLoading->levelName);
    if (cursor.ReadConst ())
    {
        ~cursor;
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"LevelLoader"}).SetExecutor<LevelLoader> ();
}
} // namespace LevelLoading
