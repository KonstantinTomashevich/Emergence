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

#include <Serialization/Yaml.hpp>

#if defined(__unix__)
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_syswm.h>
#else
#    include <SDL.h>
#    include <SDL_syswm.h>
#endif

#include <SyntaxSugar/Time.hpp>

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
        sdlTicksAfterInit = SDL_GetTicks64 ();
        sdlInitTimeNs = Emergence::Time::NanosecondsSinceStartup ();
        Emergence::ReportCriticalError ("SDL initialization", __FILE__, __LINE__);
    }

    if (!virtualFileSystem.Mount (virtualFileSystem.GetRoot (), {Emergence::VirtualFileSystem::MountSource::FILE_SYSTEM,
                                                                 "../Resources", "Resources"}))
    {
        Emergence::ReportCriticalError (EMERGENCE_BUILD_STRING ("Unable to mount resources directory into VFS!"),
                                        __FILE__, __LINE__);
    }

    if (Emergence::Resource::Provider::SourceOperationResponse result = resourceProvider.AddSource ("Resources"_us);
        result != Emergence::Resource::Provider::SourceOperationResponse::SUCCESSFUL)
    {
        Emergence::ReportCriticalError (EMERGENCE_BUILD_STRING ("Resource provider initialization error code ",
                                                                static_cast<std::uint16_t> (result)),
                                        __FILE__, __LINE__);
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
    std::uint64_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI;

    if (settings.fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    window = SDL_CreateWindow ("Platformed2dDemo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               static_cast<int> (settings.width), static_cast<int> (settings.height),
                               static_cast<SDL_WindowFlags> (windowFlags));

    SDL_SysWMinfo windowsManagerInfo;
    SDL_VERSION (&windowsManagerInfo.version);
    SDL_GetWindowWMInfo (window, &windowsManagerInfo);

#if SDL_VIDEO_DRIVER_X11
    void *nativeDisplayType = windowsManagerInfo.info.x11.display;
    void *nativeWindowHandle = (void *) (std::uintptr_t) windowsManagerInfo.info.x11.window;
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
            if (event.type == SDL_QUIT ||
                (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                 event.window.windowID == SDL_GetWindowID (window)))
            {
                running = false;
            }
            else if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && !event.key.repeat)
            {
                Emergence::Celerity::KeyboardEvent inputEvent {
                    static_cast<Emergence::Celerity::KeyCode> (event.key.keysym.sym),
                    static_cast<Emergence::Celerity::ScanCode> (event.key.keysym.scancode),
                    static_cast<Emergence::Celerity::QualifiersMask> (event.key.keysym.mod),
                    event.type == SDL_KEYDOWN ? Emergence::Celerity::KeyState::DOWN : Emergence::Celerity::KeyState::UP,
                };

                inputAccumulator->RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
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
                    event.button.x,
                    event.button.y,
                    button,
                    event.type == SDL_MOUSEBUTTONDOWN ? Emergence::Celerity::KeyState::DOWN :
                                                        Emergence::Celerity::KeyState::UP,
                    event.button.clicks,
                };

                inputAccumulator->RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                Emergence::Celerity::MouseMotionEvent inputEvent {
                    event.motion.x - event.motion.xrel,
                    event.motion.y - event.motion.yrel,
                    event.motion.x,
                    event.motion.y,
                };

                inputAccumulator->RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_MOUSEWHEEL)
            {
                Emergence::Celerity::MouseWheelEvent inputEvent {
                    event.wheel.preciseX,
                    event.wheel.preciseY,
                };

                inputAccumulator->RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
            }
            else if (event.type == SDL_TEXTINPUT)
            {
                Emergence::Celerity::TextInputEvent inputEvent;
                static_assert (sizeof (inputEvent.utf8Value) >= sizeof (event.text.text));
                strcpy (inputEvent.utf8Value.data (), event.text.text);
                inputAccumulator->RecordEvent ({SDLTicksToTime (event.key.timestamp), inputEvent});
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
