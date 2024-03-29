#define _CRT_SECURE_NO_WARNINGS

#include <cstring>
#include <filesystem>
#include <fstream>

#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Resource/Config/Loading.hpp>
#include <Celerity/Resource/Config/Messages.hpp>
#include <Celerity/Resource/Config/Test/Scenario.hpp>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
static const char *const ENVIRONMENT_ROOT = "../Resources";
static const char *const ENVIRONMENT_MOUNT = "Resources";

class Executor final : public TaskExecutorBase<Executor>
{
public:
    Executor (TaskConstructor &_constructor,
              VirtualFileSystem::Context *_virtualFileSystem,
              Resource::Provider::ResourceProvider *_resourceProvider,
              Container::Vector<Task> _tasks,
              bool *_isFinished) noexcept;

    void Execute () noexcept;

private:
    bool ExecuteTask (const Tasks::ResetEnvironment &_task) noexcept;

    bool ExecuteTask (const Tasks::LoadConfig &_task) noexcept;

    bool ExecuteTask (const Tasks::CheckUnitConfig &_task) noexcept;

    bool ExecuteTask (const Tasks::CheckBuildingConfig &_task) noexcept;

    FetchSequenceQuery fetchLoadingResponse;
    FetchAscendingRangeQuery fetchUnitConfigByAscendingId;
    FetchAscendingRangeQuery fetchBuildingConfigByAscendingId;
    InsertShortTermQuery insertLoadingRequest;

    VirtualFileSystem::Context *virtualFileSystem;
    Resource::Provider::ResourceProvider *resourceProvider;
    Container::Vector<Task> tasks;
    std::size_t currentTaskIndex = 0u;
    bool loadingRequestSent = false;
    bool *isFinished = nullptr;
};

Executor::Executor (TaskConstructor &_constructor,
                    VirtualFileSystem::Context *_virtualFileSystem,
                    Resource::Provider::ResourceProvider *_resourceProvider,
                    Container::Vector<Task> _tasks,
                    bool *_isFinished) noexcept
    : TaskExecutorBase (_constructor),

      fetchLoadingResponse (FETCH_SEQUENCE (ResourceConfigLoadedResponse)),
      fetchUnitConfigByAscendingId (FETCH_ASCENDING_RANGE (UnitConfig, id)),
      fetchBuildingConfigByAscendingId (FETCH_ASCENDING_RANGE (BuildingConfig, id)),
      insertLoadingRequest (INSERT_SHORT_TERM (ResourceConfigRequest)),

      virtualFileSystem (_virtualFileSystem),
      resourceProvider (_resourceProvider),
      tasks (std::move (_tasks)),
      isFinished (_isFinished)
{
    _constructor.MakeDependencyOf (ResourceConfigLoading::Checkpoint::STARTED);
}

void Executor::Execute () noexcept
{
    while (currentTaskIndex < tasks.size () && std::visit (
                                                   [this] (const auto &_task)
                                                   {
                                                       return ExecuteTask (_task);
                                                   },
                                                   tasks[currentTaskIndex]))
    {
        ++currentTaskIndex;
    }

    *isFinished = currentTaskIndex >= tasks.size ();
}

template <typename Type>
void SerializeConfigs (const Container::String &_folder, const Container::Vector<Type> &_configs, bool _binary)
{
    for (const Type &config : _configs)
    {
        const std::filesystem::path fullPath = std::filesystem::path {ENVIRONMENT_ROOT} / _folder /
                                               EMERGENCE_BUILD_STRING (config.id, _binary ? ".bin" : ".yaml");
        std::filesystem::create_directories (fullPath.parent_path ());

        if (_binary)
        {
            std::ofstream output {fullPath, std::ios::binary};
            Serialization::Binary::SerializeTypeName (output, Type::Reflect ().mapping.GetName ());
            Serialization::Binary::SerializeObject (output, &config, Type::Reflect ().mapping);
        }
        else
        {
            std::ofstream output {fullPath};
            Serialization::Yaml::SerializeTypeName (output, Type::Reflect ().mapping.GetName ());
            Serialization::Yaml::SerializeObject (output, &config, Type::Reflect ().mapping);
        }
    }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): Other overloads cannot be made static.
bool Executor::ExecuteTask (const Tasks::ResetEnvironment &_task) noexcept
{
    LOG ("Resetting environment...");
    [[maybe_unused]] Resource::Provider::SourceOperationResponse response =
        resourceProvider->RemoveSource (Memory::UniqueString {ENVIRONMENT_MOUNT});
    const std::filesystem::path rootPath {ENVIRONMENT_ROOT};

    if (std::filesystem::exists (rootPath))
    {
        std::filesystem::remove_all (rootPath);
    }

    virtualFileSystem->Delete (VirtualFileSystem::Entry {*virtualFileSystem, ENVIRONMENT_MOUNT}, true, true);
    std::filesystem::create_directories (rootPath);
    REQUIRE (virtualFileSystem->Mount (virtualFileSystem->GetRoot (), {VirtualFileSystem::MountSource::FILE_SYSTEM,
                                                                       ENVIRONMENT_ROOT, ENVIRONMENT_MOUNT}));

    SerializeConfigs (_task.unitConfigFolder, _task.unitConfigs, _task.useBinaryFormat);
    SerializeConfigs (_task.buildingConfigFolder, _task.buildingConfigs, _task.useBinaryFormat);

    REQUIRE (resourceProvider->AddSource (Memory::UniqueString {ENVIRONMENT_MOUNT}) ==
             Resource::Provider::SourceOperationResponse::SUCCESSFUL);
    return true;
}

bool Executor::ExecuteTask (const Tasks::LoadConfig &_task) noexcept
{
    if (!loadingRequestSent)
    {
        auto cursor = insertLoadingRequest.Execute ();
        auto *request = static_cast<ResourceConfigRequest *> (++cursor);
        request->type = _task.type;
        request->forceReload = _task.forceReload;

        LOG ("Requesting \"", _task.type.GetName (),
             "\" config to be loaded. Force reload: ", _task.forceReload ? "yes" : "no", ".");
        loadingRequestSent = true;
        return false;
    }

    for (auto cursor = fetchLoadingResponse.Execute ();
         const auto *response = static_cast<const ResourceConfigLoadedResponse *> (*cursor); ++cursor)
    {
        if (response->type == _task.type)
        {
            LOG ("Config of type \"", _task.type.GetName (), "\" is loaded!");
            loadingRequestSent = false;
            return true;
        }
    }

    return false;
}

template <typename Type>
void CheckConfig (FetchAscendingRangeQuery &_query, Container::Vector<Type> _expected)
{
    std::sort (_expected.begin (), _expected.end (),
               [] (const auto &_left, const auto &_right)
               {
                   return strcmp (*_left.id, *_right.id) < 0;
               });

    auto iterator = _expected.begin ();
    for (auto cursor = _query.Execute (nullptr, nullptr); const auto *config = static_cast<const Type *> (*cursor);
         ++cursor)
    {
        if (iterator != _expected.end ())
        {
            if (*config != *iterator)
            {
                CHECK_WITH_MESSAGE (
                    false, "Found config mismatch.\n",
                    EMERGENCE_BUILD_STRING (
                        "Loaded: ", Container::StringBuilder::ObjectPointer {config, Type::Reflect ().mapping},
                        "\nExpected: ",
                        Container::StringBuilder::ObjectPointer {&*iterator, Type::Reflect ().mapping}));
            }

            ++iterator;
        }
        else
        {
            CHECK_WITH_MESSAGE (
                false, EMERGENCE_BUILD_STRING ("Found excessive config: ", Container::StringBuilder::ObjectPointer {
                                                                               config, Type::Reflect ().mapping}));
        }
    }

    CHECK_EQUAL (iterator, _expected.end ());
}

bool Executor::ExecuteTask (const Tasks::CheckUnitConfig &_task) noexcept
{
    LOG ("Checking unit config...");
    CheckConfig (fetchUnitConfigByAscendingId, _task.unitConfigs);
    return true;
}

bool Executor::ExecuteTask (const Tasks::CheckBuildingConfig &_task) noexcept
{
    LOG ("Checking building config...");
    CheckConfig (fetchBuildingConfigByAscendingId, _task.buildingConfigs);
    return true;
}

using namespace Memory::Literals;

void ExecuteScenario (const Container::Vector<Task> &_tasks) noexcept
{
    const Container::Vector<ResourceConfigTypeMeta> typeMetas {
        {UnitConfig::Reflect ().mapping, UnitConfig::Reflect ().id},
        {BuildingConfig::Reflect ().mapping, BuildingConfig::Reflect ().id},
    };

    World world {"TestWorld"_us, WorldConfiguration {}};
    PipelineBuilder builder {world.GetRootView ()};
    bool scenarioFinished = false;

    VirtualFileSystem::Context virtualFileSystem;
    Container::MappingRegistry configTypes;
    configTypes.Register (BuildingConfig::Reflect ().mapping);
    configTypes.Register (UnitConfig::Reflect ().mapping);
    Resource::Provider::ResourceProvider resourceProvider {&virtualFileSystem, configTypes, {}};

    builder.Begin ("LoadingUpdate"_us, PipelineType::CUSTOM);
    ResourceConfigLoading::AddToLoadingPipeline (builder, &resourceProvider, typeMetas);
    builder.AddTask ("Executor"_us)
        .SetExecutor<Executor> (&virtualFileSystem, &resourceProvider, _tasks, &scenarioFinished);
    Pipeline *loadingUpdate = builder.End ();
    REQUIRE (loadingUpdate);

    while (!scenarioFinished)
    {
        loadingUpdate->Execute ();
    }
}
} // namespace Emergence::Celerity::Test
