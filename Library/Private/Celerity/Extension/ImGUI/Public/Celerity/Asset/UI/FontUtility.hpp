#pragma once

#include <imgui.h>

#include <Render/Backend/Texture.hpp>

namespace Emergence::Celerity
{
Render::Backend::Texture BakeFontAtlas (ImFontAtlas *_atlas);
} // namespace Emergence::Celerity
