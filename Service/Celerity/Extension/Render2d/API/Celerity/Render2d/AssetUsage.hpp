#pragma once

#include <Celerity/Asset/AssetReferenceBinding.hpp>

namespace Emergence::Celerity
{
/// \brief Registers Render2d objects asset usage for asset management routine.
void GetRender2dAssetUsage (AssetReferenceBindingList &_output) noexcept;
} // namespace Emergence::Celerity
