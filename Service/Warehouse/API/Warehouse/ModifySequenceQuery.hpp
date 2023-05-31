#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to gain readwrite access to all objects of selected type.
class ModifySequenceQuery final
{
public:
    /// \brief Provides readwrite access to all objects without any guaranteed order.
    class Cursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

    private:
        /// Prepared query constructs cursors.
        friend class ModifySequenceQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 3u);

        explicit Cursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (ModifySequenceQuery, Cursor);

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));

    explicit ModifySequenceQuery (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::Warehouse
