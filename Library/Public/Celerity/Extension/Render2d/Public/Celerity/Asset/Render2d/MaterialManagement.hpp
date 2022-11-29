#pragma once

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::MaterialManagement
{
/// \brief Adds task for Material asset loading and unloading in normal update pipeline.
/// \details Inserted into asset loading, therefore has no specific checkpoints.
///
/// \param _materialRootPaths Known root folders for material search.
/// \param _shaderRootPaths Known root folders for shader search.
/// \param _maxLoadingTimePerFrameNs Maximum time per frame allocated for Material2d loading.
/// \param _eventMap Event map generated as a result of asset events binding.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_materialRootPaths,
                        const Container::Vector<Memory::UniqueString> &_shaderRootPaths,
                        uint64_t _maxLoadingTimePerFrameNs,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept;
} // namespace Emergence::Celerity::MaterialManagement
