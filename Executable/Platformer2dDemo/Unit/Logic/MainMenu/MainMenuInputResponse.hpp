#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <Framework/GameState.hpp>

namespace MainMenuInputResponse
{
void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        const WorldStateRedirectionHandle &_redirectionHandle) noexcept;
} // namespace MainMenuInputResponse
