#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to start insertion transaction for objects with long lifetime.
class InsertLongTermQuery final
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
        /// \details Object default constructor will be called, if it has one.
        /// \invariant Previously allocated object must be initialized before next call.
        void *operator++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (Cursor);

    private:
        /// Prepared query constructs its cursors.
        friend class InsertLongTermQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 3u);

        explicit Cursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (InsertLongTermQuery, Cursor);

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));

    explicit InsertLongTermQuery (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::Warehouse
