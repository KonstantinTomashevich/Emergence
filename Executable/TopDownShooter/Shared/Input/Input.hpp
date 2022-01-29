#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/PipelineBuilder.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/Input/Input.h>
END_MUTING_WARNINGS

namespace Input
{
void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddToNormalUpdate (Urho3D::Context *_context, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Input
