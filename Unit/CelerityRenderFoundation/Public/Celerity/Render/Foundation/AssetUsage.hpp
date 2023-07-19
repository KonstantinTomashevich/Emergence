#pragma once

#include <CelerityRenderFoundationApi.hpp>

#include <Celerity/Asset/AssetReferenceBinding.hpp>

namespace Emergence::Celerity
{
/// \brief Registers render foundation objects asset usage for asset management routine.
CelerityRenderFoundationApi void GetRenderFoundationAssetUsage (AssetReferenceBindingList &_output) noexcept;
} // namespace Emergence::Celerity
