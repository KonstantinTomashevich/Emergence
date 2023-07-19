#pragma once

#include <Platformer2dDemoLogicApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

#include <Framework/GameState.hpp>

namespace MainMenuInputResponse
{
Platformer2dDemoLogicApi void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                                                 const WorldStateRedirectionHandle &_redirectionHandle) noexcept;
} // namespace MainMenuInputResponse
