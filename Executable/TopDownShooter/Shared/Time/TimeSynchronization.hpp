#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <OgreTimer.h>

namespace TimeSynchronization
{
void AddFixedUpdateTask (Ogre::Timer *_timer,
                         Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                         float _fixedFrameDurationS) noexcept;

void AddNormalUpdateTask (Ogre::Timer *_timer, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace TimeSynchronization
