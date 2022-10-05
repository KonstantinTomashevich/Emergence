#pragma once

#include <cstdint>

namespace Emergence::Pegasus::Constants::Implementation::VolumetricIndex
{
constexpr std::size_t MAX_DIMENSIONS = 3u;

constexpr std::size_t MAX_LEVELS = 16u;

constexpr float IDEAL_UNIT_TO_PARTITION_SCALE = 0.4f;

constexpr float EPSILON = 1e-5f;

#define IMPLEMENTATION_VOLUMETRIC_TREE_VARIANTS(Suffix)                                                                \
    VolumetricTree<int8_t, 1u> Suffix, VolumetricTree<int16_t, 1u> Suffix, VolumetricTree<int32_t, 1u> Suffix,         \
        VolumetricTree<int64_t, 1u> Suffix, VolumetricTree<uint8_t, 1u> Suffix, VolumetricTree<uint16_t, 1u> Suffix,   \
        VolumetricTree<uint32_t, 1u> Suffix, VolumetricTree<uint64_t, 1u> Suffix, VolumetricTree<float, 1u> Suffix,    \
        VolumetricTree<double, 1u> Suffix, VolumetricTree<int8_t, 2u> Suffix, VolumetricTree<int16_t, 2u> Suffix,      \
        VolumetricTree<int32_t, 2u> Suffix, VolumetricTree<int64_t, 2u> Suffix, VolumetricTree<uint8_t, 2u> Suffix,    \
        VolumetricTree<uint16_t, 2u> Suffix, VolumetricTree<uint32_t, 2u> Suffix, VolumetricTree<uint64_t, 2u> Suffix, \
        VolumetricTree<float, 2u> Suffix, VolumetricTree<double, 2u> Suffix, VolumetricTree<int8_t, 3u> Suffix,        \
        VolumetricTree<int16_t, 3u> Suffix, VolumetricTree<int32_t, 3u> Suffix, VolumetricTree<int64_t, 3u> Suffix,    \
        VolumetricTree<uint8_t, 3u> Suffix, VolumetricTree<uint16_t, 3u> Suffix, VolumetricTree<uint32_t, 3u> Suffix,  \
        VolumetricTree<uint64_t, 3u> Suffix, VolumetricTree<float, 3u> Suffix, VolumetricTree<double, 3u> Suffix
} // namespace Emergence::Pegasus::Constants::Implementation::VolumetricIndex
