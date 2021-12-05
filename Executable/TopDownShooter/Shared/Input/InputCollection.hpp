#pragma once

#include <MuteWarnings.hpp>

#include <Celerity/PipelineBuilder.hpp>

BEGIN_MUTING_WARNING
#include <OgreApplicationContext.h>
END_MUTING_WARNING

namespace InputCollection
{
void AddFixedUpdateTask (OgreBites::ApplicationContextBase *_application,
                         Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddNormalUpdateTask (OgreBites::ApplicationContextBase *_application,
                          Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace InputCollection
