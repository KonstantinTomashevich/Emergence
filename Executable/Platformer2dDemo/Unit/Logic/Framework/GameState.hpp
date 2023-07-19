#pragma once

#include <Platformer2dDemoLogicApi.hpp>

#include <functional>

#include <Celerity/Input/FrameInputAccumulator.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/World.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

class GameState;

using ModuleInitializer =
    std::function<void (GameState &, Emergence::Celerity::World &, Emergence::Celerity::WorldView &)>;

struct Platformer2dDemoLogicApi ModuleDefinition final
{
    Emergence::Memory::UniqueString coreViewName;
    Emergence::Celerity::WorldViewConfig coreViewConfig;
    ModuleInitializer initializer;
};

struct Platformer2dDemoLogicApi WorldStateDefinition final
{
    Emergence::Memory::UniqueString name;
    Emergence::Container::Vector<ModuleDefinition> modules {Emergence::Memory::Profiler::AllocationGroup::Top ()};
};

class Platformer2dDemoLogicApi WorldStateRedirectionHandle final
{
public:
    void RequestRedirect (Emergence::Memory::UniqueString _targetWorldStateName) noexcept;

private:
    friend class GameState;

    WorldStateRedirectionHandle (GameState *_gameState) noexcept;

    GameState *gameState = nullptr;
};

class Platformer2dDemoLogicApi ViewDropHandle final
{
public:
    void RequestViewDrop (Emergence::Celerity::WorldView *_view) noexcept;

private:
    friend class GameState;

    ViewDropHandle (GameState *_gameState) noexcept;

    GameState *gameState = nullptr;
};

class Platformer2dDemoLogicApi GameState final
{
public:
    static const Emergence::Memory::UniqueString TERMINATION_REDIRECT;

    GameState (Emergence::Resource::Provider::ResourceProvider *_resourceProvider,
               const Emergence::Celerity::WorldConfiguration &_worldConfiguration,
               const ModuleInitializer &_rootViewInitializer) noexcept;

    ~GameState () = default;

    GameState (const GameState &_other) = delete;

    GameState (GameState &&_other) = delete;

    void AddWorldStateDefinition (const WorldStateDefinition &_definition) noexcept;

    WorldStateRedirectionHandle ConstructWorldStateRedirectionHandle () noexcept;

    ViewDropHandle ConstructViewDropHandle () noexcept;

    Emergence::Celerity::FrameInputAccumulator *GetFrameInputAccumulator () noexcept;

    Emergence::Resource::Provider::ResourceProvider *GetResourceProvider () const noexcept;

    void ExecuteFrame () noexcept;

    [[nodiscard]] bool IsTerminated () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (GameState);

private:
    friend class WorldStateRedirectionHandle;
    friend class ViewDropHandle;

    Emergence::Celerity::World world;
    Emergence::Celerity::FrameInputAccumulator frameInputAccumulator;
    Emergence::Resource::Provider::ResourceProvider *resourceProvider;

    Emergence::Container::HashMap<Emergence::Memory::UniqueString, WorldStateDefinition> worldStates {
        Emergence::Memory::Profiler::AllocationGroup::Top ()};

    Emergence::Container::Vector<Emergence::Celerity::WorldView *> currentStateModuleRootViews {
        Emergence::Memory::Profiler::AllocationGroup::Top ()};

    Emergence::Memory::UniqueString requestedRedirect;
    Emergence::Container::Vector<Emergence::Celerity::WorldView *> viewsToDrop {
        Emergence::Memory::Profiler::AllocationGroup::Top ()};
};
