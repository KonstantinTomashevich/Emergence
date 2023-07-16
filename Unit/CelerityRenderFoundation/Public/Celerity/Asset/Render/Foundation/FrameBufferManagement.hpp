#pragma once

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/PipelineBuilder.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

namespace Emergence::Celerity::FrameBufferManagement
{
/// \brief Adds task for FrameBuffer destruction whet it is no longer referenced.
/// \details Frame buffers can only be created through ManualFrameBufferConstructor.
///          Inserted into asset loading, therefore has no specific checkpoints.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::FrameBufferManagement
