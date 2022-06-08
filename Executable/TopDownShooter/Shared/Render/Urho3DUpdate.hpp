#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/PipelineBuilder.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/Core/Context.h>
END_MUTING_WARNINGS

namespace Urho3DUpdate
{
void AddToNormalUpdate (Urho3D::Context *_context, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Urho3DUpdate
