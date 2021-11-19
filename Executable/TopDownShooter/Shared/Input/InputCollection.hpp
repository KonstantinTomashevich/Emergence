#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <OgreApplicationContext.h>

namespace InputCollection
{
void AddFixedUpdateTask (OgreBites::ApplicationContext *_application,
                         Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddNormalUpdateTask (OgreBites::ApplicationContext *_application,
                          Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace InputCollection
