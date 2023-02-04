#pragma once

#include <Framework/GameState.hpp>

namespace Modules::Root
{
Emergence::Celerity::WorldViewConfig GetViewConfig () noexcept;

void Initializer (GameState &_gameState,
                  Emergence::Celerity::World &_world,
                  Emergence::Celerity::WorldView &_rootView) noexcept;
} // namespace Modules::Root
