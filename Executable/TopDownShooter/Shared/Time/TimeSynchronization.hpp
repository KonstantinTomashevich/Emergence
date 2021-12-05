#pragma once

#include <Celerity/PipelineBuilder.hpp>

#pragma warning(push, 0)
#include <OgreTimer.h>
#pragma warning(pop)

namespace TimeSynchronization
{
void AddFixedUpdateTasks (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddNormalUpdateTasks (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace TimeSynchronization
