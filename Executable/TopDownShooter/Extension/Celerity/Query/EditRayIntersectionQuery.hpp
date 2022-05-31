#pragma once

#include <Celerity/Query/ModifyQueryWrapper.hpp>

#include <Warehouse/ModifyRayIntersectionQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_DECLARATION (RayIntersectionQuery, Warehouse::Ray _ray, float _maxDistance);
} // namespace Emergence::Celerity
