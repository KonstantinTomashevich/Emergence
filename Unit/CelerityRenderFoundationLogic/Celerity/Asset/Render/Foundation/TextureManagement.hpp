#pragma once

#include <CelerityRenderFoundationLogicApi.hpp>

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/PipelineBuilder.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

namespace Emergence::Celerity::TextureManagement
{
/// \brief Adds task for Texture asset loading and unloading in normal update pipeline.
/// \details Inserted into asset loading, therefore has no specific checkpoints.
///
/// \param _eventMap Event map generated as a result of asset events binding.
CelerityRenderFoundationLogicApi void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                                                         Resource::Provider::ResourceProvider *_resourceProvider,
                                                         const AssetReferenceBindingEventMap &_eventMap) noexcept;
} // namespace Emergence::Celerity::TextureManagement
