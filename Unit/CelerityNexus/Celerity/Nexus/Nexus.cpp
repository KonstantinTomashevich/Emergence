#include <filesystem>

#include <Assert/Assert.hpp>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Nexus/Nexus.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/WorldSingleton.hpp>

#include <Log/Log.hpp>

#include <SDL3/SDL_loadso.h>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence
{
namespace Celerity
{
Memory::Profiler::AllocationGroup GetNexusRootAllocationGroup () noexcept
{
    static Memory::Profiler::AllocationGroup group {Memory::Profiler::AllocationGroup::Top (), "CelerityNexus"_us};
    return group;
}

NexusNode::~NexusNode () noexcept
{
    for (NexusNode *child : children)
    {
        child->~NexusNode ();
        nexus->nodeHeap.Release (child, sizeof (NexusNode));
    }

    if (this != nexus->GetRootNode ())
    {
        nexus->world.DropView (view);
    }
}

const Nexus *NexusNode::GetNexus () const noexcept
{
    return nexus;
}

Nexus *NexusNode::GetNexus () noexcept
{
    return nexus;
}

Memory::UniqueString NexusNode::GetName () const noexcept
{
    return view->GetName ();
}

void NexusNode::ScheduleDrop () noexcept
{
    dropScheduled = true;
}

void NexusNode::ScheduleBootstrap (const NexusNodeBootstrap &_bootstrap) noexcept
{
    lastBootstrap.attemptToPreserveOldData = _bootstrap.attemptToPreserveOldData;
    lastBootstrap.pipelines.clear ();
    lastBootstrap.pipelines.reserve (_bootstrap.pipelines.size ());

    for (const TypedPipelineName &pipeline : _bootstrap.pipelines)
    {
        lastBootstrap.pipelines.emplace_back (pipeline);
    }

    bootstrapScheduled = true;
}

NexusNode::NexusNode (Nexus *_nexus, WorldView *_view) noexcept
    : nexus (_nexus),
      view (_view)
{
}

void NexusNode::ServeRequests () noexcept
{
    if (bootstrapScheduled)
    {
        Bootstrap (nexus->logicLibrary.GetHandle ());
    }

    for (auto iterator = children.begin (); iterator != children.end ();)
    {
        NexusNode *child = *iterator;
        if (child->dropScheduled)
        {
            child->~NexusNode ();
            nexus->nodeHeap.Release (child, sizeof (NexusNode));
            iterator = children.erase (iterator);
            continue;
        }

        child->ServeRequests ();
        ++iterator;
    }
}

void NexusNode::Bootstrap (void *_logicHandle) noexcept
{
    if (lastBootstrap.attemptToPreserveOldData)
    {
        view->GetLocalRegistry ().SetGarbageCollectionEnabled (false);
    }

    ReloadPipelines (_logicHandle);
    if (lastBootstrap.attemptToPreserveOldData)
    {
        view->GetLocalRegistry ().SetGarbageCollectionEnabled (true);
    }

    bootstrapScheduled = false;
}

void NexusNode::ReloadPipelines (void *_logicHandle) noexcept
{
    view->RemoveAllPipelines ();
    PipelineBuilder builder {view};

    for (const TypedPipelineName &pipelineName : lastBootstrap.pipelines)
    {
        auto registrar = reinterpret_cast<Nexus::PipelineRegistrarFunction> (SDL_LoadFunction (
            _logicHandle, EMERGENCE_BUILD_STRING (Nexus::PIPELINE_REGISTRAR_PREFIX, pipelineName.name)));

        if (!registrar)
        {
            EMERGENCE_LOG (ERROR, "CelerityNexus: Failed to find pipeline registrar \"", pipelineName.name, "\".");
            continue;
        }

        if (!builder.Begin (pipelineName.name, pipelineName.type))
        {
            EMERGENCE_LOG (ERROR, "CelerityNexus: Failed to start building pipeline \"", pipelineName.name, "\".");
            continue;
        }

        registrar (this, builder);
        if (!builder.End ())
        {
            EMERGENCE_LOG (ERROR, "CelerityNexus: Failed to finish building pipeline \"", pipelineName.name, "\".");
        }
    }
}

void NexusNode::ReloadLogic (void *_logicHandle) noexcept
{
    EMERGENCE_LOG (INFO, "CelerityNexus: Reloading logic for node \"", GetName (), "\".");
    view->GetLocalRegistry ().SetGarbageCollectionEnabled (false);
    ReloadPipelines (_logicHandle);
    view->GetLocalRegistry ().SetGarbageCollectionEnabled (true);

    for (NexusNode *child : children)
    {
        child->ReloadLogic (_logicHandle);
    }
}

const NexusBootstrap::Reflection &NexusBootstrap::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (NexusBootstrap);
        EMERGENCE_MAPPING_REGISTER_REGULAR (modelPath);
        EMERGENCE_MAPPING_REGISTER_REGULAR (logicPath);
        EMERGENCE_MAPPING_REGISTER_REGULAR (hotReloadEnabled);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

NexusLibrary::NexusLibrary (Container::Utf8String _path, bool _copyLibrary) noexcept
    : sourcePath (std::move (_path))
{
    if (_copyLibrary)
    {
        // For now, we only support libraries in the same directory for hot reload.
        EMERGENCE_ASSERT (std::filesystem::exists (sourcePath));
        EMERGENCE_ASSERT (sourcePath.find ('/') == std::string::npos);
        EMERGENCE_ASSERT (sourcePath.find ('\\') == std::string::npos);

        copyPath = EMERGENCE_BUILD_STRING ("HotReload0/", sourcePath);
        std::size_t attemptIndex = 1u;

        while (std::filesystem::exists (copyPath))
        {
            copyPath = EMERGENCE_BUILD_STRING ("HotReload", attemptIndex, "/", sourcePath);
            ++attemptIndex;
        }

        std::error_code errorCode;
        std::filesystem::create_directories (EMERGENCE_BUILD_STRING ("HotReload", attemptIndex - 1u), errorCode);

        if (errorCode)
        {
            Emergence::ReportCriticalError (
                EMERGENCE_BUILD_STRING ("CelerityNexus: Failed to create subdirectory for hot reload."), __FILE__,
                __LINE__);
        }

        if (!std::filesystem::copy_file (sourcePath, copyPath))
        {
            Emergence::ReportCriticalError (
                EMERGENCE_BUILD_STRING ("CelerityNexus: Failed to copy shared library \"", sourcePath, "\" to \"",
                                        copyPath, "\" for internal usage."),
                __FILE__, __LINE__);
        }
    }

    // We add current path because some OSes require full path due to non-local search.
    if (!(handle =
              SDL_LoadObject (EMERGENCE_BUILD_STRING (std::filesystem::current_path ().generic_string ().c_str (), "/",
                                                      copyPath.empty () ? sourcePath.c_str () : copyPath.c_str ()))))
    {
        Emergence::ReportCriticalError (
            EMERGENCE_BUILD_STRING ("CelerityNexus: Failed to load shared library \"", sourcePath, "\", SDL error: \"",
                                    SDL_GetError (), "\"."),
            __FILE__, __LINE__);
    }
}

NexusLibrary::NexusLibrary (NexusLibrary &&_other) noexcept
    : sourcePath (std::move (_other.sourcePath)),
      copyPath (std::move (_other.copyPath)),
      handle (_other.handle)
{
    _other.handle = nullptr;
}

NexusLibrary::~NexusLibrary () noexcept
{
    if (handle)
    {
        SDL_UnloadObject (handle);
    }

    if (!copyPath.empty ())
    {
        std::filesystem::remove (copyPath.c_str ());
    }
}

const Container::Utf8String &NexusLibrary::GetSourcePath () const noexcept
{
    return sourcePath;
}

bool NexusLibrary::HasNeverVersion () const noexcept
{
    if (!copyPath.empty ())
    {
        std::error_code errorCode;
        const std::chrono::file_clock::time_point lastWriteTime =
            std::filesystem::last_write_time (sourcePath, errorCode);

        if (errorCode)
        {
            // Source is possibly deleted: deletion happens when copying starts.
            return false;
        }

        if (lastWriteTime <= std::filesystem::last_write_time (copyPath, errorCode) || errorCode)
        {
            // Not actually newer.
            return false;
        }

        using namespace std::chrono_literals;
        constexpr std::chrono::file_clock::duration SAFE_TIME_TO_WAIT = 50ms;
        // Ensure that copy operation cannot be in progress.
        return lastWriteTime + SAFE_TIME_TO_WAIT < std::chrono::file_clock::now ();
    }

    return false;
}

void *NexusLibrary::GetHandle () const noexcept
{
    return handle;
}

NexusLibrary &NexusLibrary::operator= (NexusLibrary &&_other) noexcept
{
    if (this != &_other)
    {
        this->~NexusLibrary ();
        new (this) NexusLibrary (std::move (_other));
    }

    return *this;
}

const Memory::UniqueString Nexus::MODEL_INIT_NAME {"InitModel"};

const Memory::UniqueString Nexus::LOGIC_INIT_NAME {"InitLogic"};

const Memory::UniqueString Nexus::EVENT_REGISTRAR_NAME {"RegisterAllEvents"};

const Memory::UniqueString Nexus::PIPELINE_REGISTRAR_PREFIX {"BuildPipeline"};

Nexus::Nexus (const NexusBootstrap &_boostrap,
              Memory::UniqueString _worldName,
              const WorldConfiguration &_worldConfiguration,
              void *_userContext) noexcept
    : userContext (_userContext),
      hotReloadEnabled (_boostrap.hotReloadEnabled),
      modelLibrary (_boostrap.modelPath, _boostrap.hotReloadEnabled),
      logicLibrary (_boostrap.logicPath, _boostrap.hotReloadEnabled),
      nodeHeap (Memory::Profiler::AllocationGroup {GetNexusRootAllocationGroup (), "NexusNodes"_us}),
      world (_worldName, _worldConfiguration),
      rootNode (this, world.GetRootView ()),
      modifyWorld (world.GetRootView ()->GetLocalRegistry ().ModifySingleton (WorldSingleton::Reflect ().mapping))
{
    InitModel (modelLibrary.GetHandle ());
    InitLogic (logicLibrary.GetHandle ());
    ExecuteEventRegistrar (modelLibrary.GetHandle ());
}

const NexusNode *Nexus::GetRootNode () const noexcept
{
    return &rootNode;
}

NexusNode *Nexus::GetRootNode () noexcept
{
    return &rootNode;
}

NexusNode *Nexus::CreateChildNode (NexusNode *_parent, Memory::UniqueString _name) noexcept
{
    EMERGENCE_ASSERT (_parent);
    auto *newNode = new (nodeHeap.Acquire (sizeof (NexusNode), alignof (NexusNode)))
        NexusNode {this, world.CreateView (_parent->view, _name)};
    _parent->children.emplace_back (newNode);
    return newNode;
}

void Nexus::Update () noexcept
{
    rootNode.ServeRequests ();
    world.Update ();

    if (hotReloadEnabled)
    {
        HotReloadIfNeeded ();
    }
}

void *Nexus::GetUserContext () const noexcept
{
    return userContext;
}

void Nexus::InitModel (void *_modelHandle) noexcept
{
    EMERGENCE_LOG (INFO, "CelerityNexus: Calling model initializer.");
    auto function = reinterpret_cast<ModelInitFunction> (SDL_LoadFunction (_modelHandle, *MODEL_INIT_NAME));

    if (!function)
    {
        Emergence::ReportCriticalError (
            EMERGENCE_BUILD_STRING ("CelerityNexus: Unable to find init function in model library."), __FILE__,
            __LINE__);
    }

    function (this);
}

void Nexus::InitLogic (void *_logicHandle) noexcept
{
    EMERGENCE_LOG (INFO, "CelerityNexus: Calling logic initializer.");
    auto function = reinterpret_cast<ModelInitFunction> (SDL_LoadFunction (_logicHandle, *LOGIC_INIT_NAME));

    if (!function)
    {
        Emergence::ReportCriticalError (
            EMERGENCE_BUILD_STRING ("CelerityNexus: Unable to find init function in logic library."), __FILE__,
            __LINE__);
    }

    function (this);
}

void Nexus::HotReloadIfNeeded () noexcept
{
    const bool hasNewModel = modelLibrary.HasNeverVersion ();
    const bool hasNewLogic = logicLibrary.HasNeverVersion ();
    const bool goingForHotReload = hasNewModel || hasNewLogic;

    if (!goingForHotReload)
    {
        return;
    }

    auto worldCursor = modifyWorld.Execute ();
    auto *worldSingleton = static_cast<WorldSingleton *> (*worldCursor);

    if (worldSingleton->contextEscapeCounter > 0u)
    {
        if (worldSingleton->contextEscapeAllowed)
        {
            EMERGENCE_LOG (
                INFO,
                "CelerityNexus: Unable to start hot reload due to context escapes. Waiting till all of them return.");
        }

        worldSingleton->contextEscapeAllowed = false;

        // Wait till all escapes shut down.
        return;
    }

    Container::Optional<NexusLibrary> newModelLibrary;
    if (hasNewModel)
    {
        EMERGENCE_LOG (INFO, "CelerityNexus: Hot reloading model library.");
        newModelLibrary.emplace (modelLibrary.GetSourcePath (), true);
    }

    Container::Optional<NexusLibrary> newLogicLibrary;
    if (hasNewLogic)
    {
        EMERGENCE_LOG (INFO, "CelerityNexus: Hot reloading logic library.");
        newLogicLibrary.emplace (logicLibrary.GetSourcePath (), true);
    }

    if (hasNewModel)
    {
        InitModel (newModelLibrary->GetHandle ());
        InitLogic (newLogicLibrary->GetHandle ());
        ExecuteEventRegistrar (newModelLibrary->GetHandle ());
        rootNode.ReloadLogic (newLogicLibrary ? newLogicLibrary->GetHandle () : logicLibrary.GetHandle ());
        ReloadModel (newModelLibrary->GetHandle ());
    }
    else if (hasNewLogic)
    {
        InitLogic (newLogicLibrary->GetHandle ());
        rootNode.ReloadLogic (newLogicLibrary->GetHandle ());
    }

    if (hasNewModel)
    {
        modelLibrary = std::move (newModelLibrary.value ());
    }

    if (hasNewLogic)
    {
        logicLibrary = std::move (newLogicLibrary.value ());
    }

    worldSingleton->contextEscapeAllowed = true;
}

void Nexus::ExecuteEventRegistrar (void *_modelHandle) noexcept
{
    EMERGENCE_LOG (INFO, "CelerityNexus: Executing event registrar.");
    EventRegistrar eventRegistrar {&world, true};
    auto function = reinterpret_cast<EventRegistrarFunction> (SDL_LoadFunction (_modelHandle, *EVENT_REGISTRAR_NAME));

    if (!function)
    {
        Emergence::ReportCriticalError (
            EMERGENCE_BUILD_STRING ("CelerityNexus: Unable to find event registrar in model library."), __FILE__,
            __LINE__);
    }

    function (this, eventRegistrar);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is an experimental API stub anyway.
void Nexus::ReloadModel ([[maybe_unused]] void *_modelHandle) noexcept
{
    ReportCriticalError ("CelerityNexus: Model reloading is not yet supported!", __FILE__, __LINE__);
    // TODO: We need to patch all the patches so they point to the right mapping.
}
} // namespace Celerity

namespace Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<Celerity::TypedPipelineName>::Get () noexcept
{
    return Memory::Profiler::AllocationGroup {Celerity::GetNexusRootAllocationGroup (),
                                              UniqueString {"BootstrapPipelineInfo"}};
}
} // namespace Memory
} // namespace Emergence
