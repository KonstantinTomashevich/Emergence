#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <OgreApplicationContext.h>

namespace InputCollection
{
void AddFixedUpdateTask (OgreBites::ApplicationContextBase *_application,
                         Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddNormalUpdateTask (OgreBites::ApplicationContextBase *_application,
                          Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace InputCollection
