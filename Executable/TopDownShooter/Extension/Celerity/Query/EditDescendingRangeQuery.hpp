#pragma once

#include <Celerity/Query/ModifyQueryWrapper.hpp>

#include <Warehouse/ModifyDescendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_DECLARATION (DescendingRangeQuery, Warehouse::Bound _min, Warehouse::Bound _max);
} // namespace Emergence::Celerity