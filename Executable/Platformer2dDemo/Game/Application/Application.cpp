#include <filesystem>
#include <fstream>

#include <Application/Application.hpp>

#include <Assert/Assert.hpp>

#include <Log/Log.hpp>

#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceManagement.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialManagement.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/AssetUsage.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/Events.hpp>
#include <Celerity/Render/2d/Render2dSingleton.hpp>
#include <Celerity/Render/2d/Rendering2d.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/AssetUsage.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>

#include <Math/Constants.hpp>

#include <Render/Backend/Configuration.hpp>

#include <Serialization/Yaml.hpp>

#include <SDL.h>
#include <SDL_syswm.h>

using namespace Emergence::Memory::Literals;

// Silence error about absent allocation group for asset paths array.
// TODO: Rework that later while refactoring demo.
EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (Emergence::Memory::UniqueString)

namespace Emergence::Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<Emergence::Memory::UniqueString>::Get () noexcept
{
    return Profiler::AllocationGroup {Profiler::AllocationGroup::Top (), "UnhandledUniqueStringRefs"_us};
}
} // namespace Emergence::Memory

struct DemoScenarioExecutor final : public Emergence::Celerity::TaskExecutorBase<DemoScenarioExecutor>
{
public:
    DemoScenarioExecutor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchTimeSingleton;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::FetchSingletonQuery fetchAssetManager;
    Emergence::Celerity::ModifySingletonQuery modifyRender;

    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;
    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertSprite;

    Emergence::Celerity::EditValueQuery editUniformVector4fByAssetIdAndName;

    bool initializationDone = false;
};

DemoScenarioExecutor::DemoScenarioExecutor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTimeSingleton (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      fetchAssetManager (FETCH_SINGLETON (Emergence::Celerity::AssetManagerSingleton)),
      modifyRender (MODIFY_SINGLETON (Emergence::Celerity::Render2dSingleton)),

      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),
      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertSprite (INSERT_LONG_TERM (Emergence::Celerity::Sprite2dComponent)),

      editUniformVector4fByAssetIdAndName (
          EDIT_VALUE_2F (Emergence::Celerity::UniformVector4fValue, assetId, uniformName))
{
    _constructor.DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::DETACHMENT_DETECTION_FINISHED);
    _constructor.DependOn (Emergence::Celerity::AssetManagement::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
}

void DemoScenarioExecutor::Execute () noexcept
{
    if (!initializationDone)
    {
        auto worldCursor = fetchWorld.Execute ();
        const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

        auto assetManagerCursor = fetchAssetManager.Execute ();
        const auto *assetManager =
            static_cast<const Emergence::Celerity::AssetManagerSingleton *> (*assetManagerCursor);

        auto renderCursor = modifyRender.Execute ();
        auto *render = static_cast<Emergence::Celerity::Render2dSingleton *> (*renderCursor);

        auto transformCursor = insertTransform.Execute ();
        auto cameraCursor = insertCamera.Execute ();
        auto spriteCursor = insertSprite.Execute ();

        auto *cameraTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
        cameraTransform->SetObjectId (world->GenerateId ());
        cameraTransform->SetVisualLocalTransform ({{0.0f, 0.0f}, 0.0f, {1.0f, 1.0f}});

        auto *camera = static_cast<Emergence::Celerity::Camera2dComponent *> (++cameraCursor);
        camera->objectId = cameraTransform->GetObjectId ();
        camera->halfOrthographicSize = 5.0f;

        auto viewportCursor = insertViewport.Execute ();
        auto *viewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);

        viewport->name = "GameWorld"_us;
        viewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
        viewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
        viewport->clearColor = 0xAAAAFFFF;

        auto worldPassCursor = insertWorldPass.Execute ();
        auto *worldPass = static_cast<Emergence::Celerity::World2dRenderPass *> (++worldPassCursor);

        worldPass->name = viewport->name;
        worldPass->cameraObjectId = camera->objectId;

        for (std::size_t index = 0u; index < 20u; ++index)
        {
            auto *spriteTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
            spriteTransform->SetObjectId (world->GenerateId ());
            spriteTransform->SetVisualLocalTransform (
                {{-10.0f + static_cast<float> (index), -1.5f}, 0.0f, {1.0f, 1.0f}});

            auto *sprite = static_cast<Emergence::Celerity::Sprite2dComponent *> (++spriteCursor);
            sprite->objectId = spriteTransform->GetObjectId ();
            sprite->spriteId = render->GenerateSprite2dId ();
            sprite->assetUserId = assetManager->GenerateAssetUserId ();

            sprite->materialInstanceId = "Earth"_us;
            sprite->uv = {{0.0f, 0.0f}, {1.0f, 1.0f}};
            sprite->halfSize = {0.5f, 0.5f};
            sprite->layer = 2u;
        }

        auto *spriteTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
        spriteTransform->SetObjectId (world->GenerateId ());

        auto *sprite = static_cast<Emergence::Celerity::Sprite2dComponent *> (++spriteCursor);
        sprite->objectId = spriteTransform->GetObjectId ();
        sprite->spriteId = render->GenerateSprite2dId ();
        sprite->assetUserId = assetManager->GenerateAssetUserId ();

        sprite->materialInstanceId = "CrateLoading"_us;
        sprite->uv = {{0.0f, 0.0f}, {1.0f, 1.0f}};
        sprite->halfSize = {3.0f, 3.0f};
        sprite->layer = 0u;

        initializationDone = true;
    }

    auto timeCursor = fetchTimeSingleton.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    struct
    {
        Emergence::Memory::UniqueString assetId = "CrateLoading"_us;
        Emergence::Memory::UniqueString uniformName = "angle"_us;
    } query;

    if (auto uniformCursor = editUniformVector4fByAssetIdAndName.Execute (&query);
        auto *uniform = static_cast<Emergence::Celerity::UniformVector4fValue *> (*uniformCursor))
    {
        uniform->value.x = static_cast<float> (time->normalTimeNs % 1000000000u) * 1e-9f * 2.0f * Emergence::Math::PI;
    }
}

static Emergence::Celerity::AssetReferenceBindingList GetAssetReferenceBindingList ()
{
    Emergence::Celerity::AssetReferenceBindingList binding {Emergence::Celerity::GetAssetBindingAllocationGroup ()};
    Emergence::Celerity::GetRender2dAssetUsage (binding);
    Emergence::Celerity::GetRenderFoundationAssetUsage (binding);
    return binding;
}

static Emergence::Memory::Profiler::EventObserver StartMemoryRecording (
    Emergence::Memory::Recording::StreamSerializer &_serializer, std::ostream &_output)
{
    auto [capturedRoot, observer] = Emergence::Memory::Profiler::Capture::Start ();
    _serializer.Begin (&_output, capturedRoot);
    return std::move (observer);
}

WindowBackend::~WindowBackend () noexcept
{
    if (window)
    {
        Emergence::Render::Backend::Shutdown ();
        SDL_DestroyWindow (window);
    }

    SDL_Quit ();
}

void WindowBackend::Init (const Settings &_settings) noexcept
{
    uint64_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI;

    if (_settings.fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    window = SDL_CreateWindow ("Platformed2dDemo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               static_cast<int> (_settings.width), static_cast<int> (_settings.height),
                               static_cast<SDL_WindowFlags> (windowFlags));

    SDL_SysWMinfo windowsManagerInfo;
    SDL_VERSION (&windowsManagerInfo.version);
    SDL_GetWindowWMInfo (window, &windowsManagerInfo);

#if SDL_VIDEO_DRIVER_X11
    void *nativeDisplayType = windowsManagerInfo.info.x11.display;
    void *nativeWindowHandle = (void *) (uintptr_t) windowsManagerInfo.info.x11.window;
#elif SDL_VIDEO_DRIVER_COCOA
    void *nativeDisplayType = nullptr;
    void *nativeWindowHandle = windowsManagerInfo.info.cocoa.window;
#elif SDL_VIDEO_DRIVER_WINDOWS
    void *nativeDisplayType = nullptr;
    void *nativeWindowHandle = windowsManagerInfo.info.win.window;
#elif SDL_VIDEO_DRIVER_VIVANTE
    void *nativeDisplayType = windowsManagerInfo.info.vivante.display;
    void *nativeWindowHandle = windowsManagerInfo.info.vivante.window;
#endif

    Emergence::Render::Backend::Config config;
    config.width = _settings.width;
    config.height = _settings.height;
    config.vsync = _settings.vsync;
    Emergence::Render::Backend::Init (config, nativeWindowHandle, nativeDisplayType, false);
}

SDL_Window *WindowBackend::GetWindow () const noexcept
{
    return window;
}

Application::Application () noexcept
    : memoryEventOutput ("MemoryRecording.track", std::ios::binary),
      memoryEventObserver (StartMemoryRecording (memoryEventSerializer, memoryEventOutput))
{
    Emergence::SetIsAssertInteractive (true);
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        Emergence::ReportCriticalError ("SDL initialization", __FILE__, __LINE__);
    }
}

void Application::Run () noexcept
{
    LoadSettings ();
    InitWindow ();
    InitWorld ();
    EventLoop ();
}

void Application::LoadSettings () noexcept
{
    std::filesystem::path settingsPath = "Settings.yaml";
    if (std::filesystem::exists (settingsPath))
    {
        EMERGENCE_LOG (INFO, "Application: Loading settings...");
        std::ifstream input {settingsPath};
        Emergence::Serialization::FieldNameLookupCache cache {Settings::Reflect ().mapping};

        if (!Emergence::Serialization::Yaml::DeserializeObject (input, &settings, cache))
        {
            EMERGENCE_LOG (INFO, "Application: Failed to load settings, falling back to default.");
            settings = {};
        }
    }
    else
    {
        EMERGENCE_LOG (INFO, "Application: Saving default settings...");
        std::ofstream output {settingsPath};
        Emergence::Serialization::Yaml::SerializeObject (output, &settings, Settings::Reflect ().mapping);
    }
}

void Application::InitWindow () noexcept
{
    EMERGENCE_LOG (INFO, "Application: Initializing window...");
    windowBackend.Init (settings);
}

void Application::InitWorld () noexcept
{
    Emergence::Celerity::AssetReferenceBindingEventMap assetReferenceBindingEventMap;

    {
        Emergence::Celerity::EventRegistrar registrar {&world};
        Emergence::Celerity::RegisterAssemblyEvents (registrar);
        assetReferenceBindingEventMap =
            Emergence::Celerity::RegisterAssetEvents (registrar, GetAssetReferenceBindingList ());
        Emergence::Celerity::RegisterRender2dEvents (registrar);
        Emergence::Celerity::RegisterRenderFoundationEvents (registrar);
        Emergence::Celerity::RegisterTransform2dEvents (registrar);
        Emergence::Celerity::RegisterTransformCommonEvents (registrar);
    }

    Emergence::Celerity::PipelineBuilder pipelineBuilder {&world};

    constexpr uint64_t MAX_LOADING_TIME_NS = 10000000;
    static const Emergence::Memory::UniqueString gameMaterialInstancesPath {"../GameResources/MaterialInstances"};
    static const Emergence::Memory::UniqueString gameMaterialsPath {"../GameResources/Materials"};
    static const Emergence::Memory::UniqueString engineMaterialsPath {"../Render2dResources/Materials"};
    static const Emergence::Memory::UniqueString gameShadersPath {"../GameResources/Shaders"};
    static const Emergence::Memory::UniqueString engineShadersPath {"../Render2dResources/Shaders"};
    static const Emergence::Memory::UniqueString gameTexturesPath {"../GameResources/Textures"};
    static const Emergence::Math::AxisAlignedBox2d worldBox {{-1000.0f, -1000.0f}, {1000.0f, 1000.f}};

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    Emergence::Celerity::AssetManagement::AddToNormalUpdate (pipelineBuilder, GetAssetReferenceBindingList (),
                                                             assetReferenceBindingEventMap);
    Emergence::Celerity::TransformHierarchyCleanup::Add2dToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::MaterialInstanceManagement::AddToNormalUpdate (
        pipelineBuilder, {gameMaterialInstancesPath}, MAX_LOADING_TIME_NS, assetReferenceBindingEventMap);
    Emergence::Celerity::MaterialManagement::AddToNormalUpdate (
        pipelineBuilder, {gameMaterialsPath, engineMaterialsPath}, {gameShadersPath, engineShadersPath},
        MAX_LOADING_TIME_NS, assetReferenceBindingEventMap);
    Emergence::Celerity::RenderPipelineFoundation::AddToNormalUpdate (pipelineBuilder);
    Emergence::Celerity::Rendering2d::AddToNormalUpdate (pipelineBuilder, worldBox);
    Emergence::Celerity::TextureManagement::AddToNormalUpdate (pipelineBuilder, {gameTexturesPath}, MAX_LOADING_TIME_NS,
                                                               assetReferenceBindingEventMap);
    Emergence::Celerity::TransformVisualSync::Add2dToNormalUpdate (pipelineBuilder);
    pipelineBuilder.AddTask ("DemoScenarioExecutor"_us).SetExecutor<DemoScenarioExecutor> ();
    const bool normalPipelineRegistered = pipelineBuilder.End ();
    EMERGENCE_ASSERT (normalPipelineRegistered);
}

void Application::EventLoop () noexcept
{
    EMERGENCE_LOG (INFO, "Application: Starting event loop...");
    bool running = true;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent (&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID (windowBackend.GetWindow ()))
            {
                running = false;
            }
        }

        world.Update ();
        while (const Emergence::Memory::Profiler::Event *memoryEvent = memoryEventObserver.NextEvent ())
        {
            memoryEventSerializer.SerializeEvent (*memoryEvent);
        }
    }
}
