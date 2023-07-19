#pragma once

#include <CelerityRender2dApi.hpp>

#include <Celerity/Asset/AssetReferenceBinding.hpp>

namespace Emergence::Celerity
{
/// \brief Registers Render2d objects asset usage for asset management routine.
CelerityRender2dApi void GetRender2dAssetUsage (AssetReferenceBindingList &_output) noexcept;
} // namespace Emergence::Celerity
