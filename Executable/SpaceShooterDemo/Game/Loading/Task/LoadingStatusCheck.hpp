#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace LoadingStatusCheck
{
void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                           bool *_loadingFinishedOutput) noexcept;
} // namespace LoadingStatusCheck
