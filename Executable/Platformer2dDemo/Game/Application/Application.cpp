#include <filesystem>
#include <fstream>

#include <Application/Application.hpp>
#include <Application/Constants.hpp>

#include <Assert/Assert.hpp>

#include <Log/Log.hpp>

#include <Celerity/Render2d/BackendApi.hpp>

#include <Serialization/Yaml.hpp>

#include <SDL.h>
#include <SDL_syswm.h>

Application::Application () noexcept
{
    Emergence::SetIsAssertInteractive (true);
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        Emergence::ReportCriticalError ("SDL initialization", __FILE__, __LINE__);
    }
}

Application::~Application () noexcept
{
    if (window)
    {
        Emergence::Celerity::Render2dBackend::Shutdown ();
        SDL_DestroyWindow (window);
    }

    SDL_Quit ();
}

void Application::Run () noexcept
{
    LoadSettings ();
    InitWindow ();
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
    uint64_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI;

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

    Emergence::Celerity::Render2dBackendConfig config;
    config.width = settings.width;
    config.height = settings.height;
    config.vsync = settings.vsync;
    Emergence::Celerity::Render2dBackend::Init (config, nativeWindowHandle, nativeDisplayType, true);
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
                event.window.windowID == SDL_GetWindowID (window))
            {
                running = false;
            }
        }
    }
}
