#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Celerity/Asset/AssetReferenceBinding.hpp>

namespace Emergence::Celerity
{
/// \brief Registers render foundation objects asset usage for asset management routine.
CelerityRenderFoundationModelApi void GetRenderFoundationAssetUsage (AssetReferenceBindingList &_output) noexcept;
} // namespace Emergence::Celerity
