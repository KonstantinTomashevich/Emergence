#pragma once

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Texture2dManagement
{
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_textureRootPaths,
                        uint64_t _maxLoadingTimePerFrameNs,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept;
} // namespace Emergence::Celerity::Texture2dManagement
