#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to gain thread safe readonly access to all objects of selected type.
class FetchSequenceQuery final
{
public:
    /// \brief Provides thread safe readonly access to all objects without any guaranteed order.
    class Cursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

    private:
        /// Prepared query constructs cursors.
        friend class FetchSequenceQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);

        explicit Cursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    EMERGENCE_READONLY_PREPARED_QUERY_OPERATIONS (FetchSequenceQuery, Cursor);

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);

    explicit FetchSequenceQuery (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};
} // namespace Emergence::Warehouse
