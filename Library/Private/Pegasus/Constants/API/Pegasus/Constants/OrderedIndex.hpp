#pragma once

#include <cstdint>

#include <Pegasus/Constants/Implementation/OrderedIndex.hpp>

namespace Emergence::Pegasus::Constants::OrderedIndex
{
/// \brief If `changedRecordsCount / allRecordsCount > thisConstants`, OrderedIndex
/// will fully resort records storage instead of reinserting changed records one by one.
constexpr float MINIMUM_CHANGED_RECORDS_RATIO_TO_TRIGGER_FULL_RESORT =
    Implementation::OrderedIndex::MINIMUM_CHANGED_RECORDS_RATIO_TO_TRIGGER_FULL_RESORT;
} // namespace Emergence::Pegasus::Constants::OrderedIndex
