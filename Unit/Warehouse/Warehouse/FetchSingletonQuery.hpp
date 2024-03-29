#pragma once

#include <WarehouseApi.hpp>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to gain thread safe readonly access to singleton instance.
class WarehouseApi FetchSingletonQuery final
{
public:
    /// \brief Provides thread safe readonly access to singleton instance.
    class WarehouseApi Cursor final
    {
    public:
        Cursor (const Cursor &_other) noexcept;

        Cursor (Cursor &&_other) noexcept;

        ~Cursor () noexcept;

        /// \return Singleton instance.
        const void *operator* () const noexcept;

        /// Assigning cursors looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (Cursor);

    private:
        /// Prepared query constructs its cursors.
        friend class FetchSingletonQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));

        explicit Cursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    EMERGENCE_READONLY_PREPARED_QUERY_OPERATIONS (FetchSingletonQuery, Cursor);

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));

    explicit FetchSingletonQuery (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::Warehouse
