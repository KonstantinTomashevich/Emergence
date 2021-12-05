#pragma once

#include <MuteWarnings.hpp>

#include <Celerity/PipelineBuilder.hpp>

BEGIN_MUTING_WARNING
#include <OgreTimer.h>
END_MUTING_WARNING

namespace TimeSynchronization
{
void AddFixedUpdateTasks (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddNormalUpdateTasks (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace TimeSynchronization
