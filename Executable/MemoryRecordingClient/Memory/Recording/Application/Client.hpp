#pragma once

#include <Container/Optional.hpp>

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

    [[nodiscard]] const TrackHolder &GetTrackHolder () const noexcept;

    void OpenTrack (const char *_fileName) noexcept;

    [[nodiscard]] float GetSelectedTimeS () const noexcept;

    void SelectTime (float _seconds) noexcept;

    Client &operator= (const Client &_other) = delete;

    Client &operator= (Client &&_other) = delete;

private:
    friend int Main (int, char **);

    int Run () noexcept;

    void UpdateSelectedTime ()  noexcept;

    [[nodiscard]]  float GetEventTime (const Track::EventIterator &_iterator) const noexcept;

    bool initializedSuccessfully = true;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    TrackHolder trackHolder;
    UI ui;

    Container::Optional<float> requestedTimeS = 0.0f;
};
} // namespace Emergence::Memory::Recording::Application
