#pragma once

#include <fstream>

#include <Application/Settings.hpp>

#include <Celerity/Input/FrameInputAccumulator.hpp>
#include <Celerity/World.hpp>

#include <Memory/Recording/StreamSerializer.hpp>

class GameState;
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

    void InitGameState () noexcept;

    void EventLoop () noexcept;

    [[nodiscard]] uint64_t SDLTicksToTime (uint64_t _ticks) const noexcept;

    Settings settings;
    GameState *gameState = nullptr;
    Emergence::Memory::Heap gameStateHeap {Emergence::Memory::Profiler::AllocationGroup {
        Emergence::Memory::Profiler::AllocationGroup::Root (), Emergence::Memory::UniqueString {"GameState"}}};
    SDL_Window *window = nullptr;

    std::ofstream memoryEventOutput;
    Emergence::Memory::Recording::StreamSerializer memoryEventSerializer;
    Emergence::Memory::Profiler::EventObserver memoryEventObserver;

    uint64_t sdlInitTimeNs = 0u;
    uint64_t sdlTicksAfterInit = 0u;
};
