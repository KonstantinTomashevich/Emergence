#pragma once

#include <cstdint>

#include <Pegasus/Constants/Implementation/VolumetricIndex.hpp>

namespace Emergence::Pegasus::Constants::VolumetricIndex
{
constexpr std::size_t MAX_DIMENSIONS = Implementation::VolumetricIndex::MAX_DIMENSIONS;

constexpr std::array <std::size_t, MAX_DIMENSIONS> LEVELS = Implementation::VolumetricIndex::LEVELS;

static_assert (MAX_DIMENSIONS > 0u);
} // namespace Emergence::Pegasus::Constants::VolumetricIndex
