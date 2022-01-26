#pragma once

#include <SDL.h>

#include <Memory/Recording/Application/TrackHolder.hpp>
#include <Memory/Recording/Application/UI.hpp>

namespace Emergence::Memory::Recording::Application
{
class Client final
{
public:
    Client () noexcept;

    Client (const Client &_other) = delete;

    Client (Client &&_other) = delete;

    ~Client () noexcept;

    [[nodiscard]] TrackHolder &GetTrackHolder () noexcept;

    Client &operator= (const Client &_other) = delete;

    Client &operator= (Client &&_other) = delete;

private:
    friend int Main (int /*unused*/, char ** /*unused*/);

    int Run () noexcept;

    bool initializedSuccessfully = true;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    TrackHolder trackHolder;
    UI ui;
};
} // namespace Emergence::Memory::Recording::Application
