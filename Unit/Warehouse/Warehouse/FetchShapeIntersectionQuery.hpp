#pragma once

#include <WarehouseApi.hpp>

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/Dimension.hpp>
#include <Warehouse/Parameter.hpp>
#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to gain thread safe readonly access to
///        all objects that intersect with given shape on selected dimensions.
class WarehouseApi FetchShapeIntersectionQuery final
{
public:
    /// \brief Provides tread safe readonly access to objects that intersect with given shape.
    /// \details There is no guarantied object order.
    class WarehouseApi Cursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

    private:
        /// Prepared query constructs cursors.
        friend class FetchShapeIntersectionQuery;

        /// Cursor implementation could copy Shape inside to be more cache coherent and Shape could contain doubles,
        /// which are 8-byte long on all architectures. Therefore we use std::uint64_t as base size type.
        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 46u);

        explicit Cursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    EMERGENCE_READONLY_PREPARED_QUERY_OPERATIONS (FetchShapeIntersectionQuery, Cursor, Shape _shape);

    [[nodiscard]] DimensionIterator DimensionBegin () const noexcept;

    [[nodiscard]] DimensionIterator DimensionEnd () const noexcept;

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);

    explicit FetchShapeIntersectionQuery (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::Warehouse
