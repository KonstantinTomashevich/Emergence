#pragma once

#include <CelerityUIApi.hpp>

#include <Celerity/Asset/AssetReferenceBinding.hpp>

namespace Emergence::Celerity
{
/// \brief Registers UI objects asset usage for asset management routine.
CelerityUIApi void GetUIAssetUsage (AssetReferenceBindingList &_output) noexcept;
} // namespace Emergence::Celerity
