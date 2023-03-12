#pragma once

#include <Framework/GameState.hpp>

namespace Modules::Platformer
{
Emergence::Memory::UniqueString GetName () noexcept;

Emergence::Celerity::WorldViewConfig GetViewConfig () noexcept;

void Initializer (GameState &_gameState,
                            Emergence::Celerity::World &_world,
                            Emergence::Celerity::WorldView &_view) noexcept;
} // namespace Modules::Platformer
