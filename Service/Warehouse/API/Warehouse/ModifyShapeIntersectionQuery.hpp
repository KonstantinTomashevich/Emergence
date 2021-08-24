#pragma once

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/Dimension.hpp>
#include <Warehouse/Parameter.hpp>
#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to gain readwrite access to
///        all objects that intersect with given shape on selected dimensions.
class ModifyShapeIntersectionQuery final
{
public:
    /// \brief Provides readwrite access to objects that intersect with given shape.
    /// \details There is no guarantied object order.
    class Cursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

    private:
        /// Prepared query constructs cursors.
        friend class ModifyShapeIntersectionQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 21u);

        explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (ModifyShapeIntersectionQuery, Cursor, const Shape _shape);

    DimensionIterator DimensionBegin () const noexcept;

    DimensionIterator DimensionEnd () const noexcept;

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);

    explicit ModifyShapeIntersectionQuery (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};
} // namespace Emergence::Warehouse
