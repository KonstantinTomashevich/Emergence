#pragma once

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/PipelineBuilder.hpp>

#include <ResourceProvider/ResourceProvider.hpp>

namespace Emergence::Celerity::TextureManagement
{
/// \brief Adds task for Texture asset loading and unloading in normal update pipeline.
/// \details Inserted into asset loading, therefore has no specific checkpoints.
///
/// \param _eventMap Event map generated as a result of asset events binding.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        ResourceProvider::ResourceProvider *_resourceProvider,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept;
} // namespace Emergence::Celerity::TextureManagement
