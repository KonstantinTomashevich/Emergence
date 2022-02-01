#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/PipelineBuilder.hpp>

BEGIN_MUTING_WARNINGS
#include <OgreApplicationContext.h>
END_MUTING_WARNINGS

namespace Input
{
void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddToNormalUpdate (OgreBites::ApplicationContextBase *_application,
                        Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Input
