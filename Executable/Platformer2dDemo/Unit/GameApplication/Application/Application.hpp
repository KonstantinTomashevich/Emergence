#pragma once

#include <fstream>

#include <Application/Settings.hpp>

#include <Celerity/Nexus/Nexus.hpp>

#include <Core/NexusUserContext.hpp>

#include <Memory/Recording/StreamSerializer.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

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

    void InitVirtualFileSystem () noexcept;

    void InitNexus () noexcept;

    void EventLoop () noexcept;

    [[nodiscard]] std::uint64_t SDLTicksToTime (std::uint64_t _ticks) const noexcept;

    Settings settings;

    Emergence::Celerity::Nexus *nexus = nullptr;
    NexusUserContext nexusUserContext;
    Emergence::Memory::Heap nexusHeap {Emergence::Memory::Profiler::AllocationGroup {
        Emergence::Memory::Profiler::AllocationGroup::Root (), Emergence::Memory::UniqueString {"Nexus"}}};

    SDL_Window *window = nullptr;

    std::ofstream memoryEventOutput;
    Emergence::Memory::Recording::StreamSerializer memoryEventSerializer;
    Emergence::Memory::Profiler::EventObserver memoryEventObserver;

    std::uint64_t sdlInitTimeNs = 0u;
    std::uint64_t sdlTicksAfterInit = 0u;
};
