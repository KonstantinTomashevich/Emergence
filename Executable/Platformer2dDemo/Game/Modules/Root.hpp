#pragma once

#include <Framework/GameState.hpp>

Emergence::Celerity::WorldViewConfig GetRootModuleViewConfig () noexcept;

void RootModuleInitializer (GameState &_gameState,
                            Emergence::Celerity::World &_world,
                            Emergence::Celerity::WorldView &_rootView) noexcept;
