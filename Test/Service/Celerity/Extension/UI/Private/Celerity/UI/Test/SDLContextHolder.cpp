#include <Celerity/UI/Test/SDLContextHolder.hpp>

#include <Render/Backend/Configuration.hpp>

#include <SDL.h>
#include <SDL_syswm.h>

namespace Emergence::Celerity::Test
{
void ContextHolder::Frame () noexcept
{
    static ContextHolder contextHolder;
    SDL_Event event;

    while (SDL_PollEvent (&event))
    {
        // Just poll all events...
    }
}

ContextHolder::ContextHolder () noexcept
{
    uint64_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI;
    window = SDL_CreateWindow ("Celerity::Render tests", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               static_cast<int> (WIDTH), static_cast<int> (HEIGHT),
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

    Render::Backend::Config config;
    config.width = WIDTH;
    config.height = HEIGHT;
    config.vsync = true;
    Render::Backend::Init (config, nativeWindowHandle, nativeDisplayType, false);
}

ContextHolder::~ContextHolder () noexcept
{
    if (window)
    {
        Render::Backend::Shutdown ();
        SDL_DestroyWindow (window);
    }

    SDL_Quit ();
}
} // namespace Emergence::Celerity::Test
