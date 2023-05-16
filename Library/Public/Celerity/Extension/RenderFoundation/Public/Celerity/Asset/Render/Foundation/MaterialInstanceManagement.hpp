#pragma once

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::MaterialInstanceManagement
{
/// \brief Adds task for MaterialInstance asset loading and unloading in normal update pipeline.
/// \details Inserted into asset loading, therefore has no specific checkpoints.
///
/// \param _materialInstanceRootPaths Known root folders for material instance search.
/// \param _maxLoadingTimePerFrameNs Maximum time per frame allocated for MaterialInstance loading.
/// \param _eventMap Event map generated as a result of asset events binding.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_materialInstanceRootPaths,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept;
} // namespace Emergence::Celerity::MaterialInstanceManagement
