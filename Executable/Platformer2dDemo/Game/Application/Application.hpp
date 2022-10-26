#pragma once

#include <Application/Settings.hpp>

struct SDL_Window;

class Application final
{
public:
    Application () noexcept;

    Application (const Application &_other) = delete;

    Application (Application &&_other) = delete;

    ~Application () noexcept;

    void Run () noexcept;

    Application &operator= (const Application &_other) = delete;

    Application &operator= (Application &&_other) = delete;

private:
    void LoadSettings () noexcept;

    void InitWindow () noexcept;

    void EventLoop () noexcept;

    SDL_Window *window = nullptr;
    Settings settings;
};
