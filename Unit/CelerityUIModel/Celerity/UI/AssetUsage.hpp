#pragma once

#include <CelerityUIModelApi.hpp>

#include <Celerity/Asset/AssetReferenceBinding.hpp>

namespace Emergence::Celerity
{
/// \brief Registers UI objects asset usage for asset management routine.
CelerityUIModelApi void GetUIAssetUsage (AssetReferenceBindingList &_output) noexcept;
} // namespace Emergence::Celerity
