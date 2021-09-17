#pragma once

#include <cstdint>

namespace Emergence::Pegasus::Constants::Implementation::VolumetricIndex
{
constexpr std::size_t MAX_DIMENSIONS = 3u;

constexpr std::array<std::size_t, MAX_DIMENSIONS> LEVELS = {16, 8, 6};

constexpr float EPSILON = 1e-7f;
} // namespace Emergence::Pegasus::Constants::Implementation::VolumetricIndex
