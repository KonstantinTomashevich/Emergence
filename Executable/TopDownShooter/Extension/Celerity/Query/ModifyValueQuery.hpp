#pragma once

#include <Celerity/Query/ModifyQueryWrapper.hpp>

#include <Warehouse/ModifyValueQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_DECLARATION (ModifyValueQuery, Warehouse::ValueSequence _values);
} // namespace Emergence::Celerity
