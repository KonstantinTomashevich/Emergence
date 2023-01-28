#include <Framework/GameState.hpp>

using namespace Emergence::Memory::Literals;

void WorldStateRedirectionHandle::RequestRedirect (Emergence::Memory::UniqueString _targetWorldStateName) noexcept
{
    gameState->requestedRedirect = _targetWorldStateName;
}

WorldStateRedirectionHandle::WorldStateRedirectionHandle (GameState *_gameState) noexcept
    : gameState (_gameState)
{
    EMERGENCE_ASSERT (_gameState);
}

GameState::GameState (const Emergence::Celerity::WorldConfiguration &_worldConfiguration,
                      const ModuleInitializer &_rootViewInitializer) noexcept
    : world ("GameStateWorld"_us, _worldConfiguration)
{
    _rootViewInitializer (*this, world, *world.GetRootView ());
}

void GameState::AddWorldStateDefinition (const WorldStateDefinition &_definition) noexcept
{
    EMERGENCE_ASSERT (!worldStates.contains (_definition.name));
    worldStates[_definition.name] = _definition;
}

WorldStateRedirectionHandle GameState::ConstructWorldStateRedirectionHandle () noexcept
{
    return {this};
}

Emergence::Celerity::FrameInputAccumulator *GameState::GetFrameInputAccumulator () noexcept
{
    return &frameInputAccumulator;
}

void GameState::ExecuteFrame () noexcept
{
    if (*requestedRedirect)
    {
        for (Emergence::Celerity::WorldView *view : currentStateModuleRootViews)
        {
            world.DropView (view);
        }

        currentStateModuleRootViews.clear ();
        EMERGENCE_ASSERT (worldStates.contains (requestedRedirect));
        WorldStateDefinition &definition = worldStates[requestedRedirect];
        requestedRedirect = {};
        currentStateModuleRootViews.reserve (definition.modules.size ());

        for (const ModuleDefinition &moduleDefinition : definition.modules)
        {
            Emergence::Celerity::WorldView *view =
                world.CreateView (world.GetRootView (), moduleDefinition.coreViewName, moduleDefinition.coreViewConfig);
            currentStateModuleRootViews.emplace_back (view);
            moduleDefinition.initializer (*this, world, *view);
        }
    }

    world.Update ();
    frameInputAccumulator.Clear ();
}
