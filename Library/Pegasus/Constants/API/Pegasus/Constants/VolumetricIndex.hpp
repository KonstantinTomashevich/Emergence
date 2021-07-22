#pragma once

#include <array>
#include <cstdint>

#include <Pegasus/Constants/Implementation/VolumetricIndex.hpp>

namespace Emergence::Pegasus::Constants::VolumetricIndex
{
/// \brief Maximum supported count of dimensions for VolumetricIndex.
constexpr std::size_t MAX_DIMENSIONS = Implementation::VolumetricIndex::MAX_DIMENSIONS;

/// \brief Maximum subdivision levels for each dimension count.
constexpr std::array <std::size_t, MAX_DIMENSIONS> LEVELS = Implementation::VolumetricIndex::LEVELS;

/// \brief Used to check floating point values equality.
constexpr float EPSILON = Implementation::VolumetricIndex::EPSILON;

static_assert (MAX_DIMENSIONS > 0u);
} // namespace Emergence::Pegasus::Constants::VolumetricIndex
