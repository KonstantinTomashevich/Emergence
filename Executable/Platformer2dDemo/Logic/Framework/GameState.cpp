#include <Framework/GameState.hpp>

using namespace Emergence::Memory::Literals;

void WorldStateRedirectionHandle::RequestRedirect (Emergence::Memory::UniqueString _targetWorldStateName) noexcept
{
    // Assert that there is no redirection overlap.
    EMERGENCE_ASSERT (!*gameState->requestedRedirect);
    gameState->requestedRedirect = _targetWorldStateName;
}

WorldStateRedirectionHandle::WorldStateRedirectionHandle (GameState *_gameState) noexcept
    : gameState (_gameState)
{
    EMERGENCE_ASSERT (gameState);
}

void ViewDropHandle::RequestViewDrop (Emergence::Celerity::WorldView *_view) noexcept
{
    gameState->viewsToDrop.emplace_back (_view);
}

ViewDropHandle::ViewDropHandle (GameState *_gameState) noexcept
    : gameState (_gameState)
{
    EMERGENCE_ASSERT (gameState);
}

const Emergence::Memory::UniqueString GameState::TERMINATION_REDIRECT {"Termination"};

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

ViewDropHandle GameState::ConstructViewDropHandle () noexcept
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
        if (requestedRedirect == TERMINATION_REDIRECT)
        {
            // Application should handle termination redirect and exit.
            EMERGENCE_ASSERT (false);
            return;
        }

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

    for (Emergence::Celerity::WorldView *view : viewsToDrop)
    {
        world.DropView (view);
    }

    viewsToDrop.clear ();
}

bool GameState::IsTerminated () const noexcept
{
    return requestedRedirect == TERMINATION_REDIRECT;
}
