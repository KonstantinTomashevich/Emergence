#pragma once

#include <Celerity/Query/ModifyQueryWrapper.hpp>

#include <Warehouse/ModifyAscendingRangeQuery.hpp>

namespace Emergence::Celerity
{
WAREHOUSE_MODIFY_QUERY_WRAPPER_DECLARATION (ModifyAscendingRangeQuery, Warehouse::Bound _min, Warehouse::Bound _max);
} // namespace Emergence::Celerity
