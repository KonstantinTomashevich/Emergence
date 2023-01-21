#pragma once

#include <fstream>

#include <Application/Settings.hpp>

#include <Celerity/Input/FrameInputAccumulator.hpp>
#include <Celerity/World.hpp>

#include <Memory/Recording/StreamSerializer.hpp>

// TODO: Currently this demo is used for quick testing, therefore everything is added into Application file.
//       Should be refactored into real demo later.

struct SDL_Window;

class WindowBackend final
{
public:
    WindowBackend () noexcept = default;

    WindowBackend (const WindowBackend &_other) = delete;

    WindowBackend (WindowBackend &&_other) = delete;

    ~WindowBackend () noexcept;

    void Init (const Settings &_settings) noexcept;

    [[nodiscard]] SDL_Window *GetWindow () const noexcept;

    WindowBackend &operator= (const WindowBackend &_other) = delete;

    WindowBackend &operator= (WindowBackend &&_other) = delete;

private:
    SDL_Window *window = nullptr;
};

class Application final
{
public:
    Application () noexcept;

    Application (const Application &_other) = delete;

    Application (Application &&_other) = delete;

    ~Application () noexcept = default;

    void Run () noexcept;

    Application &operator= (const Application &_other) = delete;

    Application &operator= (Application &&_other) = delete;

private:
    void LoadSettings () noexcept;

    void InitWindow () noexcept;

    void InitWorld () noexcept;

    void EventLoop () noexcept;

    [[nodiscard]] uint64_t SDLTicksToTime (uint64_t _ticks) const noexcept;

    Settings settings;
    WindowBackend windowBackend;
    Emergence::Celerity::FrameInputAccumulator inputAccumulator;
    Emergence::Celerity::World world {Emergence::Memory::UniqueString {"TestWorld"}, {{1.0f / 60.0f}}};

    std::ofstream memoryEventOutput;
    Emergence::Memory::Recording::StreamSerializer memoryEventSerializer;
    Emergence::Memory::Profiler::EventObserver memoryEventObserver;

    uint64_t sdlInitTimeNs = 0u;
    uint64_t sdlTicksAfterInit = 0u;
};
