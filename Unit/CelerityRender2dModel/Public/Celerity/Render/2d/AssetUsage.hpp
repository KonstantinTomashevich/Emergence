#pragma once

#include <CelerityRender2dModelApi.hpp>

#include <Celerity/Asset/AssetReferenceBinding.hpp>

namespace Emergence::Celerity
{
/// \brief Registers Render2d objects asset usage for asset management routine.
CelerityRender2dModelApi void GetRender2dAssetUsage (AssetReferenceBindingList &_output) noexcept;
} // namespace Emergence::Celerity
