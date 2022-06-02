#pragma once

#include <Celerity/Query/QueryWrapper.hpp>

#include <Warehouse/ModifyDescendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_DECLARATION (DescendingRangeQuery,
                                                     Warehouse::Bound _min,
                                                     Warehouse::Bound _max);
} // namespace Emergence::Celerity
