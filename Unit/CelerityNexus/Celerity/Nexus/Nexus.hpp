#pragma once

#include <CelerityNexusApi.hpp>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/World.hpp>

#include <Container/Optional.hpp>
#include <Container/String.hpp>
#include <Container/TypedOrderedPool.hpp>

namespace Emergence::Celerity
{
// TODO: Currently Nexus is left as an experimental framework for hot reload support.
//       Its development is frozen due to potential breaking changes in Emergence architecture (moving from Celerity
//       to even more threading-friendly framework) or potential pause in Emergence development due to pure C related
//       research (that might trigger full migration to new pure C project).
//       Therefore, there is no documentation for now.

using namespace Memory::Literals;

class Nexus;

CelerityNexusApi Memory::Profiler::AllocationGroup GetNexusRootAllocationGroup () noexcept;

struct CelerityNexusApi TypedPipelineName final
{
    Memory::UniqueString name;
    PipelineType type = PipelineType::CUSTOM;
};

struct CelerityNexusApi NexusNodeBootstrap final
{
    bool attemptToPreserveOldData = true;
    Container::Vector<TypedPipelineName> pipelines {
        Memory::Profiler::AllocationGroup {GetNexusRootAllocationGroup (), "NexusNodeBootstrap"_us}};
};

class CelerityNexusApi NexusNode final
{
public:
    NexusNode (const NexusNode &_other) = delete;

    NexusNode (NexusNode &&_other) = delete;

    ~NexusNode () noexcept;

    [[nodiscard]] const Nexus *GetNexus () const noexcept;

    Nexus *GetNexus () noexcept;

    [[nodiscard]] Memory::UniqueString GetName () const noexcept;

    void ScheduleDrop () noexcept;

    void ScheduleBootstrap (const NexusNodeBootstrap &_bootstrap) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (NexusNode);

private:
    friend class Nexus;

    template <typename T>
    friend class Container::TypedOrderedPool;

    NexusNode (Nexus *_nexus, WorldView *_view) noexcept;

    void ServeRequests () noexcept;

    void Bootstrap (void *_logicHandle) noexcept;

    void ReloadPipelines (void *_logicHandle) noexcept;

    void ReloadLogic (void *_logicHandle) noexcept;

    Nexus *nexus = nullptr;
    WorldView *view = nullptr;

    bool dropScheduled = false;
    bool bootstrapScheduled = false;
    NexusNodeBootstrap lastBootstrap;

    Container::Vector<NexusNode *> children {
        Memory::Profiler::AllocationGroup {GetNexusRootAllocationGroup (), "NexusNodeChildren"_us}};
};

struct CelerityNexusApi NexusBootstrap final
{
    Container::Utf8String modelPath;
    Container::Utf8String logicPath;
    bool hotReloadEnabled = true;

    struct Reflection final
    {
        StandardLayout::FieldId modelPath;
        StandardLayout::FieldId logicPath;
        StandardLayout::FieldId hotReloadEnabled;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

class CelerityNexusApi NexusLibrary final
{
public:
    NexusLibrary (Container::Utf8String _path, bool _copyLibrary) noexcept;

    NexusLibrary (const NexusLibrary &_other) = delete;

    NexusLibrary (NexusLibrary &&_other) noexcept;

    ~NexusLibrary () noexcept;

    [[nodiscard]] const Container::Utf8String &GetSourcePath () const noexcept;

    [[nodiscard]] bool HasNeverVersion () const noexcept;

    [[nodiscard]] void *GetHandle () const noexcept;

    NexusLibrary &operator= (const NexusLibrary &_other) = delete;

    NexusLibrary &operator= (NexusLibrary &&_other) noexcept;

private:
    Container::Utf8String sourcePath;
    Container::Utf8String copyPath;
    void *handle = nullptr;
};

class CelerityNexusApi Nexus final
{
public:
    static const Memory::UniqueString MODEL_INIT_NAME;

    using ModelInitFunction = void (__cdecl *) (Nexus *);

    static const Memory::UniqueString LOGIC_INIT_NAME;

    using LogicInitFunction = void (__cdecl *) (Nexus *);

    static const Memory::UniqueString EVENT_REGISTRAR_NAME;

    using EventRegistrarFunction = void (__cdecl *) (Nexus *, EventRegistrar &);

    static const Memory::UniqueString PIPELINE_REGISTRAR_PREFIX;

    using PipelineRegistrarFunction = void (__cdecl *) (NexusNode *, PipelineBuilder &);

    explicit Nexus (const NexusBootstrap &_boostrap,
                    Memory::UniqueString _worldName,
                    const WorldConfiguration &_worldConfiguration,
                    void *_userContext) noexcept;

    Nexus (const Nexus &_other) = delete;

    Nexus (Nexus &&_other) = delete;

    ~Nexus () noexcept = default;

    [[nodiscard]] const NexusNode *GetRootNode () const noexcept;

    NexusNode *GetRootNode () noexcept;

    NexusNode *CreateChildNode (NexusNode *_parent, Memory::UniqueString _name) noexcept;

    void Update () noexcept;

    // TODO: Use managed Any's as user data?
    [[nodiscard]] void *GetUserContext () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Nexus);

private:
    friend class NexusNode;

    void InitModel (void *_modelHandle) noexcept;

    void InitLogic (void *_logicHandle) noexcept;

    void HotReloadIfNeeded () noexcept;

    void ExecuteEventRegistrar (void *_modelHandle) noexcept;

    void ReloadModel (void *_modelHandle) noexcept;

    void *userContext = nullptr;

    bool hotReloadEnabled = true;

    NexusLibrary modelLibrary;

    NexusLibrary logicLibrary;

    Memory::Heap nodeHeap;

    World world;

    NexusNode rootNode;

    Warehouse::ModifySingletonQuery modifyWorld;
};
} // namespace Emergence::Celerity

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (CelerityNexusApi, Emergence::Celerity::TypedPipelineName)
