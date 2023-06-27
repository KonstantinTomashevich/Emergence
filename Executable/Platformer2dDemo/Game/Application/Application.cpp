#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <Application/Application.hpp>

#include <Assert/Assert.hpp>

#include <Configuration/ResourceProviderTypes.hpp>
#include <Configuration/WorldStates.hpp>

#include <CPU/Profiler.hpp>

#include <Log/Log.hpp>

#include <Framework/GameState.hpp>

#include <Modules/MainMenu.hpp>
#include <Modules/Platformer.hpp>
#include <Modules/Root.hpp>

#include <Render/Backend/Configuration.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_syswm.h>

#include <SyntaxSugar/Time.hpp>

#undef CreateDirectory
#undef ERROR

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
      memoryEventObserver (StartMemoryRecording (memoryEventSerializer, memoryEventOutput)),
      resourceProvider (&virtualFileSystem, GetResourceTypesRegistry (), GetPatchableTypesRegistry ())
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

    const Emergence::VirtualFileSystem::Entry resourcesDirectory {
        virtualFileSystem.CreateDirectory (virtualFileSystem.GetRoot (), "Resources")};

    Emergence::VirtualFileSystem::MountConfigurationList configurationList;
    {
        Emergence::Container::Utf8String mountListPath;
        bool binary = false;

        if (std::filesystem::exists ("MountCoreResources.bin"))
        {
            mountListPath = "MountCoreResources.bin";
            binary = true;
        }
        else if (std::filesystem::exists ("MountCoreResources.yaml"))
        {
            mountListPath = "MountCoreResources.yaml";
            binary = false;
        }
        else if (std::filesystem::exists ("../MountCoreResources.bin"))
        {
            mountListPath = "../MountCoreResources.bin";
            binary = true;
        }
        else if (std::filesystem::exists ("../MountCoreResources.yaml"))
        {
            mountListPath = "../MountCoreResources.yaml";
            binary = false;
        }
        else
        {
            Emergence::ReportCriticalError ("Unable to find core resources mount list!", __FILE__, __LINE__);
        }

        std::ifstream input {mountListPath.c_str (), binary ? std::ios::binary : std::ios::in};
        if (!input)
        {
            Emergence::ReportCriticalError ("Failed to open core resources mount list!", __FILE__, __LINE__);
        }

        if (binary)
        {
            if (!Emergence::Serialization::Binary::DeserializeObject (
                    input, &configurationList, Emergence::VirtualFileSystem::MountConfigurationList::Reflect ().mapping,
                    {}))
            {
                Emergence::ReportCriticalError ("Failed to deserialize core resources mount list!", __FILE__, __LINE__);
            }
        }
        else
        {
            if (!Emergence::Serialization::Yaml::DeserializeObject (
                    input, &configurationList, Emergence::VirtualFileSystem::MountConfigurationList::Reflect ().mapping,
                    {}))
            {
                Emergence::ReportCriticalError ("Failed to deserialize core resources mount list!", __FILE__, __LINE__);
            }
        }
    }

    for (const auto &configuration : configurationList.items)
    {
        if (!virtualFileSystem.Mount (resourcesDirectory, configuration))
        {
            Emergence::ReportCriticalError (EMERGENCE_BUILD_STRING ("Failed to mount \"", configuration.sourcePath,
                                                                    "\" into \"", configuration.targetPath, "\"!"),
                                            __FILE__, __LINE__);
        }

        const Emergence::Memory::UniqueString mountedSource {EMERGENCE_BUILD_STRING (
            "Resources", Emergence::VirtualFileSystem::PATH_SEPARATOR, configuration.targetPath)};

        if (Emergence::Resource::Provider::SourceOperationResponse result = resourceProvider.AddSource (mountedSource);
            result != Emergence::Resource::Provider::SourceOperationResponse::SUCCESSFUL)
        {
            Emergence::ReportCriticalError (
                EMERGENCE_BUILD_STRING ("Resource provider initialization error code ",
                                        static_cast<std::uint16_t> (result), " while trying to add source \"",
                                        mountedSource, "\"!"),
                __FILE__, __LINE__);
        }
    }
}

Application::~Application () noexcept
{
    if (gameState)
    {
        gameState->~GameState ();
        gameStateHeap.Release (gameState, sizeof (GameState));
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
    InitGameState ();
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

void Application::InitGameState () noexcept
{
    gameState = new (gameStateHeap.Acquire (sizeof (GameState), alignof (GameState)))
        GameState {&resourceProvider,
                   {{1.0f / 120.0f, 1.0f / 60.0f, 1.0f / 30.0f}, Modules::Root::GetViewConfig ()},
                   Modules::Root::Initializer};

    WorldStateDefinition mainMenuState;
    mainMenuState.name = WorldStates::MAIN_MENU;
    mainMenuState.modules.emplace_back () = {Modules::MainMenu::GetName (), Modules::MainMenu::GetViewConfig (),
                                             Modules::MainMenu::Initializer};
    gameState->AddWorldStateDefinition (mainMenuState);

    WorldStateDefinition platformerState;
    platformerState.name = WorldStates::PLATFORMER;
    platformerState.modules.emplace_back () = {Modules::Platformer::GetName (), Modules::Platformer::GetViewConfig (),
                                               Modules::Platformer::Initializer};
    gameState->AddWorldStateDefinition (platformerState);

    gameState->ConstructWorldStateRedirectionHandle ().RequestRedirect (WorldStates::MAIN_MENU);
}

void Application::EventLoop () noexcept
{
    EMERGENCE_LOG (INFO, "Application: Starting event loop...");
    bool running = true;

    while (running)
    {
        SDL_Event event;
        Emergence::Celerity::FrameInputAccumulator *inputAccumulator = gameState->GetFrameInputAccumulator ();

        while (SDL_PollEvent (&event))
        {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID (window)))
            {
                running = false;
            }
            else if ((event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) && !event.key.repeat)
            {
                Emergence::Celerity::KeyboardEvent inputEvent {
                    static_cast<Emergence::Celerity::KeyCode> (event.key.keysym.sym),
                    static_cast<Emergence::Celerity::ScanCode> (event.key.keysym.scancode),
                    static_cast<Emergence::Celerity::QualifiersMask> (event.key.keysym.mod),
                    event.type == SDL_EVENT_KEY_DOWN ? Emergence::Celerity::KeyState::DOWN :
                                                       Emergence::Celerity::KeyState::UP,
                };

                inputAccumulator->RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                Emergence::Celerity::MouseButton button = Emergence::Celerity::MouseButton::LEFT;
                switch (event.button.button)
                {
                case SDL_BUTTON_LEFT:
                    button = Emergence::Celerity::MouseButton::LEFT;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button = Emergence::Celerity::MouseButton::MIDDLE;
                    break;
                case SDL_BUTTON_RIGHT:
                    button = Emergence::Celerity::MouseButton::RIGHT;
                    break;
                case SDL_BUTTON_X1:
                    button = Emergence::Celerity::MouseButton::X1;
                    break;
                case SDL_BUTTON_X2:
                    button = Emergence::Celerity::MouseButton::X2;
                    break;
                }

                Emergence::Celerity::MouseButtonEvent inputEvent {
                    static_cast<std::int32_t> (event.button.x),
                    static_cast<std::int32_t> (event.button.y),
                    button,
                    event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? Emergence::Celerity::KeyState::DOWN :
                                                                Emergence::Celerity::KeyState::UP,
                    event.button.clicks,
                };

                inputAccumulator->RecordEvent ({SDLTicksToTime (event.button.timestamp), inputEvent});
            }
            else if (event.type == SDL_EVENT_MOUSE_MOTION)
            {
                Emergence::Celerity::MouseMotionEvent inputEvent {
                    static_cast<std::int32_t> (event.motion.x - event.motion.xrel),
                    static_cast<std::int32_t> (event.motion.y - event.motion.yrel),
                    static_cast<std::int32_t> (event.motion.x),
                    static_cast<std::int32_t> (event.motion.y),
                };

                inputAccumulator->RecordEvent ({SDLTicksToTime (event.motion.timestamp), inputEvent});
            }
            else if (event.type == SDL_EVENT_MOUSE_WHEEL)
            {
                Emergence::Celerity::MouseWheelEvent inputEvent {
                    event.wheel.x,
                    event.wheel.y,
                };

                inputAccumulator->RecordEvent ({SDLTicksToTime (event.wheel.timestamp), inputEvent});
            }
            else if (event.type == SDL_EVENT_TEXT_INPUT)
            {
                Emergence::Celerity::TextInputEvent inputEvent;
                static_assert (sizeof (inputEvent.utf8Value) >= sizeof (event.text.text));
                strcpy (inputEvent.utf8Value.data (), event.text.text);
                inputAccumulator->RecordEvent ({SDLTicksToTime (event.text.timestamp), inputEvent});
            }
        }

        gameState->ExecuteFrame ();
        while (const Emergence::Memory::Profiler::Event *memoryEvent = memoryEventObserver.NextEvent ())
        {
            memoryEventSerializer.SerializeEvent (*memoryEvent);
        }

        running &= !gameState->IsTerminated ();
        Emergence::CPU::Profiler::MarkFrameEnd ();
    }
}

uint64_t Application::SDLTicksToTime (std::uint64_t _ticks) const noexcept
{
    return (_ticks - sdlTicksAfterInit) * 1000000u + sdlInitTimeNs;
}
