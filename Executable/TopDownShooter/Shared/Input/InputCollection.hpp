#pragma once

#include <Celerity/PipelineBuilder.hpp>

#pragma warning(push, 0)
#include <OgreApplicationContext.h>
#pragma warning(pop)

namespace InputCollection
{
void AddFixedUpdateTask (OgreBites::ApplicationContextBase *_application,
                         Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddNormalUpdateTask (OgreBites::ApplicationContextBase *_application,
                          Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace InputCollection
