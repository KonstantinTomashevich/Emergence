#pragma once

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to gain thread safe read only access to all objects
///        that are considered signaled -- they have selected value in selected field.
class FetchSignalQuery final
{
public:
    /// \brief Provides read only access to signaled objects.
    /// \details There is no guarantied object order.
    class Cursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

    private:
        /// Prepared query constructs cursors.
        friend class FetchSignalQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);

        explicit Cursor (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    EMERGENCE_READONLY_PREPARED_QUERY_OPERATIONS (FetchSignalQuery, Cursor);

    [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

    /// \details Prefer using this method instead of comparison with ::GetSignaledValue,
    ///          because it strips off insignificant bits that may arise due to memory reinterpretation.
    [[nodiscard]] bool IsSignaledValue (const std::array<uint8_t, sizeof (uint64_t)> &_value) const;

    [[nodiscard]] std::array<uint8_t, sizeof (uint64_t)> GetSignaledValue () const noexcept;

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);

    explicit FetchSignalQuery (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::Warehouse
