#pragma once

#include <Celerity/Query/ModifyQueryWrapper.hpp>

#include <Warehouse/ModifyRayIntersectionQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_DECLARATION (ModifyRayIntersectionQuery, Warehouse::Ray _ray, float _maxDistance);
} // namespace Emergence::Celerity
