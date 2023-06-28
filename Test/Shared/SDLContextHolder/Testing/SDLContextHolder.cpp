#include <Render/Backend/Configuration.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_syswm.h>

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
    std::uint64_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow ("Emergence Tests", static_cast<int> (WIDTH), static_cast<int> (HEIGHT),
                               static_cast<SDL_WindowFlags> (windowFlags));

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
