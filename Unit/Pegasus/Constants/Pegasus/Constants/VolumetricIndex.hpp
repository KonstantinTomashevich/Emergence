#pragma once

#include <array>
#include <cstdint>

#include <Pegasus/Constants/VolumetricIndexProfile.hpp>

namespace Emergence::Pegasus::Constants::VolumetricIndex
{
/// \brief Maximum supported count of dimensions for VolumetricIndex.
constexpr std::size_t MAX_DIMENSIONS = Profile::VolumetricIndex::MAX_DIMENSIONS;

/// \brief Maximum supported count of levels for volumetric trees.
constexpr std::size_t MAX_LEVELS = Profile::VolumetricIndex::MAX_LEVELS;

/// \brief Ideal scale for unit-to-partition conversion.
/// \details Actual scale might be a bit higher due to partition rounding to power of two.
constexpr float IDEAL_UNIT_TO_PARTITION_SCALE = Profile::VolumetricIndex::IDEAL_UNIT_TO_PARTITION_SCALE;

/// \brief Used to check floating point values equality.
constexpr float EPSILON = Profile::VolumetricIndex::EPSILON;

/// \brief Provides list of all volumetric tree variants for supported types and dimensions.
/// \details Suffix is added for convenient listing of enumerators.
#define VOLUMETRIC_TREE_VARIANTS(...) IMPLEMENTATION_VOLUMETRIC_TREE_VARIANTS (__VA_ARGS__)

static_assert (MAX_DIMENSIONS > 0u);
static_assert (MAX_LEVELS > 0u);
static_assert (IDEAL_UNIT_TO_PARTITION_SCALE > 0.0f);
} // namespace Emergence::Pegasus::Constants::VolumetricIndex
