#pragma once

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/KeyFieldIterator.hpp>
#include <Warehouse/Parameter.hpp>
#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to gain readwrite access to
///        all objects with requested values in selected key fields.
class ModifyValueQuery final
{
public:
    /// \brief Provides readwrite access to objects with requested values.
    class Cursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

    private:
        /// Prepared query constructs cursors.
        friend class ModifyValueQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);

        explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (ModifyValueQuery, Cursor, ValueSequence _values);

    KeyFieldIterator KeyFieldBegin () const noexcept;

    KeyFieldIterator KeyFieldEnd () const noexcept;

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);

    explicit ModifyValueQuery (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};
} // namespace Emergence::Warehouse
