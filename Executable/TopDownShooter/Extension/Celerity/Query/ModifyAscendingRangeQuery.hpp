#pragma once

#include <Celerity/Query/QueryWrapper.hpp>

#include <Warehouse/ModifyAscendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_DECLARATION (AscendingRangeQuery, Warehouse::Bound _min, Warehouse::Bound _max);
} // namespace Emergence::Celerity
