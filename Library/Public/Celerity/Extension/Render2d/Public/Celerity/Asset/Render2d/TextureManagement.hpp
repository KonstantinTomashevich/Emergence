#pragma once

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::TextureManagement
{
/// \brief Adds task for Texture asset loading and unloading in normal update pipeline.
/// \details Inserted into asset loading, therefore has no specific checkpoints.
///
/// \param _textureRootPaths Known root folders for texture search.
/// \param _maxLoadingTimePerFrameNs Maximum time per frame allocated for Texture2d loading.
/// \param _eventMap Event map generated as a result of asset events binding.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_textureRootPaths,
                        uint64_t _maxLoadingTimePerFrameNs,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept;
} // namespace Emergence::Celerity::TextureManagement