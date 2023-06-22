#include <Render/Backend/Configuration.hpp>

#if defined(__unix__)
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_syswm.h>
#else
#    include <SDL.h>
#    include <SDL_syswm.h>
#endif

#include <Testing/SDLContextHolder.hpp>

namespace Emergence::Testing
{
SDLContextHolder &SDLContextHolder::Get () noexcept
{
    static SDLContextHolder contextHolder;
    return contextHolder;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): Intentionally non-static to make refactors easier.
void SDLContextHolder::Frame () noexcept
{
    SDL_Event event;
    while (SDL_PollEvent (&event))
    {
        // Just poll all events...
    }
}

SDLContextHolder::SDLContextHolder () noexcept
{
    std::uint64_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI;
    window =
        SDL_CreateWindow ("Emergence Tests", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int> (WIDTH),
                          static_cast<int> (HEIGHT), static_cast<SDL_WindowFlags> (windowFlags));

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

    Render::Backend::Config config;
    config.width = WIDTH;
    config.height = HEIGHT;
    config.vsync = true;
    Render::Backend::Init (config, nativeWindowHandle, nativeDisplayType, false);
}

SDLContextHolder::~SDLContextHolder () noexcept
{
    if (window)
    {
        Render::Backend::Shutdown ();
        SDL_DestroyWindow (window);
    }

    SDL_Quit ();
}
} // namespace Emergence::Testing
