#pragma once

#include <cstdint>

namespace Emergence::Celerity::Implementation
{
constexpr std::size_t MAX_COPY_OUT_BLOCKS_PER_EVENT = 4u;

constexpr std::size_t MAX_TRACKED_ZONES_PER_EVENT = 4u;

constexpr std::size_t MAX_ON_CHANGE_EVENTS_PER_TYPE = 4u;

constexpr std::size_t CHANGE_TRACKER_MAX_TRACKED_ZONES = 4u;

constexpr std::size_t CHANGE_TRACKER_MAX_TRACKING_BUFFER_SIZE = 256u;
} // namespace Emergence::Celerity
