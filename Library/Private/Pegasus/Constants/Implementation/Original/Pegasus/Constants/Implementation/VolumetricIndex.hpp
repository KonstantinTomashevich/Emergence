#pragma once

#include <cstdint>

namespace Emergence::Pegasus::Constants::Implementation::VolumetricIndex
{
constexpr std::size_t MAX_DIMENSIONS = 3u;

constexpr std::size_t MAX_LEVELS = 16u;

constexpr float IDEAL_UNIT_TO_PARTITION_SCALE = 0.4f;

constexpr float EPSILON = 1e-5f;

#define IMPLEMENTATION_VOLUMETRIC_TREE_VARIANTS(...)                                                                   \
    VolumetricTree<int8_t, 1u> __VA_ARGS__, VolumetricTree<int16_t, 1u> __VA_ARGS__,                                   \
        VolumetricTree<int32_t, 1u> __VA_ARGS__, VolumetricTree<int64_t, 1u> __VA_ARGS__,                              \
        VolumetricTree<uint8_t, 1u> __VA_ARGS__, VolumetricTree<uint16_t, 1u> __VA_ARGS__,                             \
        VolumetricTree<uint32_t, 1u> __VA_ARGS__, VolumetricTree<uint64_t, 1u> __VA_ARGS__,                            \
        VolumetricTree<float, 1u> __VA_ARGS__, VolumetricTree<double, 1u> __VA_ARGS__,                                 \
        VolumetricTree<int8_t, 2u> __VA_ARGS__, VolumetricTree<int16_t, 2u> __VA_ARGS__,                               \
        VolumetricTree<int32_t, 2u> __VA_ARGS__, VolumetricTree<int64_t, 2u> __VA_ARGS__,                              \
        VolumetricTree<uint8_t, 2u> __VA_ARGS__, VolumetricTree<uint16_t, 2u> __VA_ARGS__,                             \
        VolumetricTree<uint32_t, 2u> __VA_ARGS__, VolumetricTree<uint64_t, 2u> __VA_ARGS__,                            \
        VolumetricTree<float, 2u> __VA_ARGS__, VolumetricTree<double, 2u> __VA_ARGS__,                                 \
        VolumetricTree<int8_t, 3u> __VA_ARGS__, VolumetricTree<int16_t, 3u> __VA_ARGS__,                               \
        VolumetricTree<int32_t, 3u> __VA_ARGS__, VolumetricTree<int64_t, 3u> __VA_ARGS__,                              \
        VolumetricTree<uint8_t, 3u> __VA_ARGS__, VolumetricTree<uint16_t, 3u> __VA_ARGS__,                             \
        VolumetricTree<uint32_t, 3u> __VA_ARGS__, VolumetricTree<uint64_t, 3u> __VA_ARGS__,                            \
        VolumetricTree<float, 3u> __VA_ARGS__, VolumetricTree<double, 3u> __VA_ARGS__
} // namespace Emergence::Pegasus::Constants::Implementation::VolumetricIndex
