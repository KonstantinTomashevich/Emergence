#pragma once

#include <WarehouseApi.hpp>

#include <API/Common/Iterator.hpp>

#include <StandardLayout/Field.hpp>

namespace Emergence::Warehouse
{
/// \brief Describes one dimensions for volumetric prepared queries.
struct WarehouseApi Dimension
{
    /// \brief Pointer to minimum possible value of #minBorderField.
    ///
    /// \details Values, that are less than this value will be processed as this value.
    ///          Guarantied to be copied during prepared query creation, therefore can point to stack memory.
    const void *globalMinBorder;

    /// \brief Field, that holds record minimum border value for this dimension.
    ///
    /// \invariant Field archetype is FieldArchetype::INT, FieldArchetype::UINT or FieldArchetype::FLOAT.
    StandardLayout::Field minBorderField;

    /// \brief Pointer to maximum possible value of #maxBorderField.
    ///
    /// \details Values, that are greater than this value will be processed as this value.
    ///          Guarantied to be copied during prepared query creation, therefore can point to stack memory.
    const void *globalMaxBorder;

    /// \brief Field, that holds record maximum border value for this dimension.
    ///
    /// \invariant Field archetype is FieldArchetype::INT, FieldArchetype::UINT or FieldArchetype::FLOAT.
    StandardLayout::Field maxBorderField;
};

/// \brief Provides iteration over dimensions of volumetric prepared queries.
class WarehouseApi DimensionIterator final
{
public:
    EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (DimensionIterator, Dimension);

private:
    /// Volumetric prepared queries construct dimension iterators.
    friend class FetchRayIntersectionQuery;

    friend class FetchShapeIntersectionQuery;

    friend class ModifyRayIntersectionQuery;

    friend class ModifyShapeIntersectionQuery;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);

    explicit DimensionIterator (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::Warehouse

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (WarehouseApi, Emergence::Warehouse::Dimension)
