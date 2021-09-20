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
///        all objects that intersect with given ray on selected dimensions.
class ModifyRayIntersectionQuery final
{
public:
    /// \brief Provides readwrite access to objects that intersect with given ray.
    /// \details There is no guarantied object order.
    class Cursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

    private:
        /// Prepared query constructs cursors.
        friend class ModifyRayIntersectionQuery;

        /// Cursor implementation could copy Ray inside to be more cache coherent and Ray could contain doubles,
        /// which are 8-byte long on all architectures. Therefore we use uint64_t as base size type.
        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t) * 19u);

        explicit Cursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    EMERGENCE_EDITABLE_PREPARED_QUERY_OPERATIONS (ModifyRayIntersectionQuery, Cursor, Ray _ray, float _maxDistance);

    DimensionIterator DimensionBegin () const noexcept;

    DimensionIterator DimensionEnd () const noexcept;

private:
    /// Registry constructs prepared queries.
    friend class Registry;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);

    explicit ModifyRayIntersectionQuery (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};
} // namespace Emergence::Warehouse
