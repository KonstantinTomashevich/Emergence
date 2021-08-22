#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to start insertion transaction for objects with short lifetime.
class InsertShortTermQuery final
{
public:
    /// \brief Allows user to allocate new objects and insert them into registry.
    class Cursor final
    {
    public:
        Cursor (const Cursor &_other) = delete;

        Cursor (Cursor &&_other) noexcept;

        /// \invariant Previously allocated object must be initialized before cursor destruction.
        ~Cursor () noexcept;

        /// \return Pointer to memory, allocated for the new object.
        /// \invariant Previously allocated object must be initialized before next call.
        void *operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (Cursor);

    private:
        /// Prepared query constructs its cursors.
        friend class InsertShortTermQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (InsertShortTermQuery, Cursor);

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);

    explicit InsertShortTermQuery (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};
} // namespace Emergence::Warehouse
