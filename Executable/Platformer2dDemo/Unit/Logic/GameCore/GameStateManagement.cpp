#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/WorldSingleton.hpp>

#include <Core/PipelineNames.hpp>

#include <GameCore/GameStateManagement.hpp>
#include <GameCore/GameStateSingleton.hpp>

namespace GameStateManagement
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"GameStateManagementStarted"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"GameStateManagementFinished"};

class GameStateManager final : public Emergence::Celerity::TaskExecutorBase<GameStateManager>
{
public:
    GameStateManager (Emergence::Celerity::TaskConstructor &_constructor,
                      Emergence::Celerity::NexusNode *_gameNode,
                      bool *_terminateFlag) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyGameState;
    Emergence::Celerity::ModifySingletonQuery modifyWorld;
    Emergence::Celerity::NexusNode *gameNode = nullptr;
    bool *terminateFlag = nullptr;
};

GameStateManager::GameStateManager (Emergence::Celerity::TaskConstructor &_constructor,
                                    Emergence::Celerity::NexusNode *_gameNode,
                                    bool *_terminateFlag) noexcept
    : TaskExecutorBase (_constructor),

      modifyGameState (MODIFY_SINGLETON (GameStateSingleton)),
      modifyWorld (MODIFY_SINGLETON (Emergence::Celerity::WorldSingleton)),
      gameNode (_gameNode),
      terminateFlag (_terminateFlag)
{
    EMERGENCE_ASSERT (gameNode);
}

void GameStateManager::Execute () noexcept
{
    auto gameStateCursor = modifyGameState.Execute ();
    auto *gameState = static_cast<GameStateSingleton *> (*gameStateCursor);

    auto worldCursor = modifyWorld.Execute ();
    auto *world = static_cast<Emergence::Celerity::WorldSingleton *> (*worldCursor);

    // If there is not predefined game state, start loading main menu.
    if (gameState->state == GameState::NONE)
    {
        gameState->request.state = GameState::MAIN_MENU_LOADING;
    }

    switch (gameState->request.state)
    {
    case GameState::NONE:
        break;

    case GameState::MAIN_MENU_LOADING:
        EMERGENCE_ASSERT (gameState->state == GameState::NONE || gameState->state == GameState::PLATFORMER_GAME);
        gameState->state = GameState::MAIN_MENU_LOADING;
        world->updateMode = Emergence::Celerity::WorldUpdateMode::FROZEN;
        gameNode->ScheduleBootstrap (
            {false,
             {
                 {PipelineNames::MAIN_MENU_LOADING_FIXED, Emergence::Celerity::PipelineType::FIXED},
                 {PipelineNames::MAIN_MENU_LOADING_NORMAL, Emergence::Celerity::PipelineType::NORMAL},
             }});
        break;

    case GameState::MAIN_MENU:
        EMERGENCE_ASSERT (gameState->state == GameState::MAIN_MENU_LOADING);
        gameState->state = GameState::MAIN_MENU;
        world->updateMode = Emergence::Celerity::WorldUpdateMode::SIMULATING;
        gameNode->ScheduleBootstrap (
            {true,
             {
                 {PipelineNames::MAIN_MENU_READY_FIXED, Emergence::Celerity::PipelineType::FIXED},
                 {PipelineNames::MAIN_MENU_READY_NORMAL, Emergence::Celerity::PipelineType::NORMAL},
             }});
        break;

    case GameState::PLATFORMER_LOADING:
        EMERGENCE_ASSERT (gameState->state == GameState::MAIN_MENU || gameState->state == GameState::PLATFORMER_GAME);
        gameState->state = GameState::PLATFORMER_LOADING;
        world->updateMode = Emergence::Celerity::WorldUpdateMode::FROZEN;
        gameNode->ScheduleBootstrap (
            {false,
             {
                 {PipelineNames::PLATFORMER_LOADING_FIXED, Emergence::Celerity::PipelineType::FIXED},
                 {PipelineNames::PLATFORMER_LOADING_NORMAL, Emergence::Celerity::PipelineType::NORMAL},
             }});
        break;

    case GameState::PLATFORMER_GAME:
        EMERGENCE_ASSERT (gameState->state == GameState::PLATFORMER_LOADING);
        gameState->state = GameState::PLATFORMER_GAME;
        world->updateMode = Emergence::Celerity::WorldUpdateMode::SIMULATING;
        gameNode->ScheduleBootstrap (
            {true,
             {
                 {PipelineNames::PLATFORMER_GAME_FIXED, Emergence::Celerity::PipelineType::FIXED},
                 {PipelineNames::PLATFORMER_GAME_NORMAL, Emergence::Celerity::PipelineType::NORMAL},
             }});
        break;

    case GameState::TERMINATED:
        gameState->state = GameState::TERMINATED;
        *terminateFlag = true;
        gameNode->ScheduleDrop ();
        break;
    }

    gameState->lastRequest = gameState->request;
    gameState->request.state = {};
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        Emergence::Celerity::NexusNode *_gameNode,
                        bool *_terminateFlag) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("GameStateManager"_us).SetExecutor<GameStateManager> (_gameNode, _terminateFlag);
}
} // namespace GameStateManagement
