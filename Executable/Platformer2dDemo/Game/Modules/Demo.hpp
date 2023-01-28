#pragma once

#include <Framework/GameState.hpp>

Emergence::Memory::UniqueString GetDemoModuleName () noexcept;

Emergence::Celerity::WorldViewConfig GetDemoModuleViewConfig () noexcept;

void DemoModuleInitializer (GameState &_gameState,
                            Emergence::Celerity::World &_world,
                            Emergence::Celerity::WorldView &_rootView) noexcept;
