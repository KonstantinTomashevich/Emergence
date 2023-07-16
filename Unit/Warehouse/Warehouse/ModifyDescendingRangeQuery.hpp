#pragma once

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/Parameter.hpp>
#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to gain readwrite access to
///        all objects in requested range on selected key field.
/// \details Bounds could be `nullptr`. `nullptr` bound is processed as absence of bound.
class ModifyDescendingRangeQuery final
{
public:
    /// \brief Provides readwrite access to objects in requested range.
    ///        Objects are sorted in descending order.
    class Cursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

    private:
        /// Prepared query constructs cursors.
        friend class ModifyDescendingRangeQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 3u);

        explicit Cursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (ModifyDescendingRangeQuery, Cursor, Bound _min, Bound _max);

    [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);

    explicit ModifyDescendingRangeQuery (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::Warehouse
