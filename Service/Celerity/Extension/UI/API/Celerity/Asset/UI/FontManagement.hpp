#pragma once

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::FontManagement
{
/// \brief Adds task for Font asset loading and unloading in normal update pipeline.
/// \details Inserted into asset loading, therefore has no specific checkpoints.
///
/// \param _fontRootPaths Known root folders for font search.
/// \param _maxLoadingTimePerFrameNs Maximum time per frame allocated for Font loading.
/// \param _eventMap Event map generated as a result of asset events binding.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_fontRootPaths,
                        uint64_t _maxLoadingTimePerFrameNs,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept;
} // namespace Emergence::Celerity::FontManagement
