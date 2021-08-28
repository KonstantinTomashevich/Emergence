#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Warehouse/PreparedQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Prepared query, used to gain readwrite access to singleton instance.
class ModifySingletonQuery final
{
public:
    /// \brief Provides readwrite access to singleton instance.
    class Cursor final
    {
    public:
        Cursor (const Cursor &_other) = delete;

        Cursor (Cursor &&_other) noexcept;

        ~Cursor () noexcept;

        /// \return Singleton instance.
        void *operator* () const noexcept;

        /// Assigning cursors looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (Cursor);

    private:
        /// Prepared query constructs its cursors.
        friend class ModifySingletonQuery;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit Cursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (ModifySingletonQuery, Cursor);

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);

    explicit ModifySingletonQuery (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};
} // namespace Emergence::Warehouse
