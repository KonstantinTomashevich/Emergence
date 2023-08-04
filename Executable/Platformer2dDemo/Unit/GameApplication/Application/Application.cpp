#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <Application/Application.hpp>

#include <Assert/Assert.hpp>

#include <Core/PipelineNames.hpp>

#include <CPU/Profiler.hpp>

#include <Log/Log.hpp>

#include <Render/Backend/Configuration.hpp>

#include <Serialization/Yaml.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_syswm.h>
#undef CreateDirectory
#undef ERROR

#include <Time/Time.hpp>

#include <VirtualFileSystem/Helpers.hpp>

using namespace Emergence::Memory::Literals;

static Emergence::Memory::Profiler::EventObserver StartMemoryRecording (
    Emergence::Memory::Recording::StreamSerializer &_serializer, std::ostream &_output)
{
    auto [capturedRoot, observer] = Emergence::Memory::Profiler::Capture::Start ();
    _serializer.Begin (&_output, capturedRoot);
    return std::move (observer);
}

Application::Application () noexcept
    : memoryEventOutput ("MemoryRecording.track", std::ios::binary),
      memoryEventObserver (StartMemoryRecording (memoryEventSerializer, memoryEventOutput))
{
    Emergence::Log::GlobalLogger::Init (Emergence::Log::Level::ERROR,
                                        {Emergence::Log::Sinks::StandardOut {{Emergence::Log::Level::INFO}}});
    Emergence::SetIsAssertInteractive (true);

    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        sdlTicksAfterInit = SDL_GetTicks ();
        sdlInitTimeNs = Emergence::Time::NanosecondsSinceStartup ();
        Emergence::ReportCriticalError ("SDL initialization", __FILE__, __LINE__);
    }
}

Application::~Application () noexcept
{
    if (nexus)
    {
        // We need to manually deallocate Any's before their destructors are unloaded.
        nexusUserContext.assetReferenceBindingList.Reset ();
        nexusUserContext.assetReferenceBindingEventMap.Reset ();

        nexus->~Nexus ();
        nexusHeap.Release (nexus, sizeof (Emergence::Celerity::Nexus));
    }

    if (window)
    {
        Emergence::Render::Backend::Shutdown ();
        SDL_DestroyWindow (window);
    }

    SDL_Quit ();
}

void Application::Run () noexcept
{
    LoadSettings ();
    InitWindow ();
    InitVirtualFileSystem ();
    InitNexus ();
    EventLoop ();
}

void Application::LoadSettings () noexcept
{
    std::filesystem::path settingsPath = "Settings.yaml";
    if (std::filesystem::exists (settingsPath))
    {
        EMERGENCE_LOG (INFO, "Application: Loading settings...");
        std::ifstream input {settingsPath};

        if (!Emergence::Serialization::Yaml::DeserializeObject (input, &settings, Settings::Reflect ().mapping, {}))
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
    std::uint64_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    if (settings.fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    window = SDL_CreateWindow ("Platformed2dDemo", static_cast<int> (settings.width),
                               static_cast<int> (settings.height), static_cast<SDL_WindowFlags> (windowFlags));

    SDL_SysWMinfo windowsManagerInfo;
    SDL_GetWindowWMInfo (window, &windowsManagerInfo, SDL_SYSWM_CURRENT_VERSION);

#if defined(SDL_ENABLE_SYSWM_X11)
    void *nativeDisplayType = windowsManagerInfo.info.x11.display;
    auto *nativeWindowHandle =
        reinterpret_cast<void *> (static_cast<std::uintptr_t> (windowsManagerInfo.info.x11.window));
#elif defined(SDL_ENABLE_SYSWM_COCOA)
    void *nativeDisplayType = nullptr;
    void *nativeWindowHandle = windowsManagerInfo.info.cocoa.window;
#elif defined(SDL_ENABLE_SYSWM_WINDOWS)
    void *nativeDisplayType = nullptr;
    void *nativeWindowHandle = windowsManagerInfo.info.win.window;
#elif defined(SDL_ENABLE_SYSWM_VIVANTE)
    void *nativeDisplayType = windowsManagerInfo.info.vivante.display;
    void *nativeWindowHandle = windowsManagerInfo.info.vivante.window;
#endif

    Emergence::Render::Backend::Config config;
    config.width = settings.width;
    config.height = settings.height;
    config.vsync = settings.vsync;
    Emergence::Render::Backend::Init (config, nativeWindowHandle, nativeDisplayType, false);
}

void Application::InitVirtualFileSystem () noexcept
{
    nexusUserContext.resourcesRoot = {nexusUserContext.virtualFileSystem.CreateDirectory (
        nexusUserContext.virtualFileSystem.GetRoot (), "Resources")};

    if (!Emergence::VirtualFileSystem::FetchMountConfigurationList (".", "CoreResources",
                                                                    nexusUserContext.resourcesMount) &&
        !Emergence::VirtualFileSystem::FetchMountConfigurationList ("..", "CoreResources",
                                                                    nexusUserContext.resourcesMount))
    {
        Emergence::ReportCriticalError ("Failed to fetch \"CoreResources\" mount list!", __FILE__, __LINE__);
    }

    if (!Emergence::VirtualFileSystem::MountConfigurationListAt (
            nexusUserContext.virtualFileSystem, nexusUserContext.resourcesRoot, nexusUserContext.resourcesMount))
    {
        Emergence::ReportCriticalError ("Failed to mount \"CoreResources\" mount list!", __FILE__, __LINE__);
    }
}

void Application::InitNexus () noexcept
{
    Emergence::Celerity::NexusBootstrap bootstrap;
    if (std::ifstream nexusBootstrapSibling {"CelerityNexusBootstrap.yaml"})
    {
        if (!Emergence::Serialization::Yaml::DeserializeObject (
                nexusBootstrapSibling, &bootstrap, Emergence::Celerity::NexusBootstrap::Reflect ().mapping, {}))
        {
            Emergence::ReportCriticalError ("Failed to deserialize \"CelerityNexusBootstrap.yaml\"!", __FILE__,
                                            __LINE__);
        }
    }
    else
    {
        Emergence::ReportCriticalError ("Failed to find CelerityNexus bootstrap file!", __FILE__, __LINE__);
    }

    nexus = new (nexusHeap.Acquire (sizeof (Emergence::Celerity::Nexus), alignof (Emergence::Celerity::Nexus)))
        Emergence::Celerity::Nexus {
            bootstrap, "GameWorld"_us, {{1.0f / 120.0f, 1.0f / 60.0f, 1.0f / 30.0f}}, &nexusUserContext};

    nexus->GetRootNode ()->ScheduleBootstrap (
        {false,
         {
             {PipelineNames::ROOT_NORMAL, Emergence::Celerity::PipelineType::NORMAL},
         }});

    nexusUserContext.gameStateNode = nexus->CreateChildNode (nexus->GetRootNode (), "GameState"_us);
    nexusUserContext.gameStateNode->ScheduleBootstrap (
        {false,
         {
             {PipelineNames::GAME_ROOT_NORMAL, Emergence::Celerity::PipelineType::NORMAL},
         }});

    nexusUserContext.gameWorldNode = nexus->CreateChildNode (nexusUserContext.gameStateNode, "GameWorld"_us);
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
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID (window)))
            {
                running = false;
            }
            else if ((event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) && !event.key.repeat)
            {
                Emergence::InputStorage::KeyboardEvent inputEvent {
                    static_cast<Emergence::InputStorage::KeyCode> (event.key.keysym.sym),
                    static_cast<Emergence::InputStorage::ScanCode> (event.key.keysym.scancode),
                    static_cast<Emergence::InputStorage::QualifiersMask> (event.key.keysym.mod),
                    event.type == SDL_EVENT_KEY_DOWN ? Emergence::InputStorage::KeyState::DOWN :
                                                       Emergence::InputStorage::KeyState::UP,
                };

                nexusUserContext.inputAccumulator.RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                Emergence::InputStorage::MouseButton button = Emergence::InputStorage::MouseButton::LEFT;
                switch (event.button.button)
                {
                case SDL_BUTTON_LEFT:
                    button = Emergence::InputStorage::MouseButton::LEFT;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button = Emergence::InputStorage::MouseButton::MIDDLE;
                    break;
                case SDL_BUTTON_RIGHT:
                    button = Emergence::InputStorage::MouseButton::RIGHT;
                    break;
                case SDL_BUTTON_X1:
                    button = Emergence::InputStorage::MouseButton::X1;
                    break;
                case SDL_BUTTON_X2:
                    button = Emergence::InputStorage::MouseButton::X2;
                    break;
                }

                Emergence::InputStorage::MouseButtonEvent inputEvent {
                    static_cast<std::int32_t> (event.button.x),
                    static_cast<std::int32_t> (event.button.y),
                    button,
                    event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? Emergence::InputStorage::KeyState::DOWN :
                                                                Emergence::InputStorage::KeyState::UP,
                    event.button.clicks,
                };

                nexusUserContext.inputAccumulator.RecordEvent ({SDLTicksToTime (event.button.timestamp), inputEvent});
            }
            else if (event.type == SDL_EVENT_MOUSE_MOTION)
            {
                Emergence::InputStorage::MouseMotionEvent inputEvent {
                    static_cast<std::int32_t> (event.motion.x - event.motion.xrel),
                    static_cast<std::int32_t> (event.motion.y - event.motion.yrel),
                    static_cast<std::int32_t> (event.motion.x),
                    static_cast<std::int32_t> (event.motion.y),
                };

                nexusUserContext.inputAccumulator.RecordEvent ({SDLTicksToTime (event.motion.timestamp), inputEvent});
            }
            else if (event.type == SDL_EVENT_MOUSE_WHEEL)
            {
                Emergence::InputStorage::MouseWheelEvent inputEvent {
                    event.wheel.x,
                    event.wheel.y,
                };

                nexusUserContext.inputAccumulator.RecordEvent ({SDLTicksToTime (event.wheel.timestamp), inputEvent});
            }
            else if (event.type == SDL_EVENT_TEXT_INPUT)
            {
                Emergence::InputStorage::TextInputEvent inputEvent;
                static_assert (sizeof (inputEvent.utf8Value) >= sizeof (event.text.text));
                strcpy (inputEvent.utf8Value.data (), event.text.text);
                nexusUserContext.inputAccumulator.RecordEvent ({SDLTicksToTime (event.text.timestamp), inputEvent});
            }
        }

        nexus->Update ();
        nexusUserContext.inputAccumulator.Clear ();

        while (const Emergence::Memory::Profiler::Event *memoryEvent = memoryEventObserver.NextEvent ())
        {
            memoryEventSerializer.SerializeEvent (*memoryEvent);
        }

        running &= !nexusUserContext.terminateRequested;
        Emergence::CPU::Profiler::MarkFrameEnd ();
    }
}

uint64_t Application::SDLTicksToTime (std::uint64_t _ticks) const noexcept
{
    return (_ticks - sdlTicksAfterInit) * 1000000u + sdlInitTimeNs;
}
