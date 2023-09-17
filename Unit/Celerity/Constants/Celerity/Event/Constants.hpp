#pragma once

#include <cstdint>

#include <Celerity/Event/ConstantsProfile.hpp>

namespace Emergence::Celerity
{
/// \brief Maximum amount of CopyOutBlock's per event.
/// \details For performance reasons, CopyOutBlock's are stored in inplace vectors,
///          therefore maximum amount of these blocks per event is fixed.
constexpr std::size_t MAX_COPY_OUT_BLOCKS_PER_EVENT = Profile::MAX_COPY_OUT_BLOCKS_PER_EVENT;

/// \brief Maximum amount of tracking zones (sets of adjacent fields) per event.
/// \details For performance reasons, tracking zones are stored in inplace vectors,
///          therefore maximum amount of these zones per event is fixed.
constexpr std::size_t MAX_TRACKED_ZONES_PER_EVENT = Profile::MAX_TRACKED_ZONES_PER_EVENT;

/// \brief Maximum count of OnChange events per type.
/// \details For performance reasons, ChangeTracker stores info about OnChange
///          events in inplace vector, therefore maximum count of events is fixed.
constexpr std::size_t MAX_ON_CHANGE_EVENTS_PER_TYPE = Profile::MAX_ON_CHANGE_EVENTS_PER_TYPE;

/// \brief Maximum amount of tracking zones (sets of adjacent fields) per change tracker.
/// \details For performance reasons, tracking zones are stored in inplace vectors,
///          therefore maximum amount of these zones per event is fixed.
constexpr std::size_t CHANGE_TRACKER_MAX_TRACKED_ZONES = Profile::CHANGE_TRACKER_MAX_TRACKED_ZONES;

/// \brief Capacity of change tracking buffer.
/// \details For performance reasons, change tracking buffer is stored as
///          inplace array inside change tracker, therefore its capacity is fixed.
constexpr std::size_t CHANGE_TRACKER_MAX_TRACKING_BUFFER_SIZE = Profile::CHANGE_TRACKER_MAX_TRACKING_BUFFER_SIZE;
} // namespace Emergence::Celerity
