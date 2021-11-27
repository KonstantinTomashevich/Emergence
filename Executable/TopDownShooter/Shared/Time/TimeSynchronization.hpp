#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <OgreTimer.h>

namespace TimeSynchronization
{
void AddFixedUpdateTasks (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddNormalUpdateTasks (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace TimeSynchronization
