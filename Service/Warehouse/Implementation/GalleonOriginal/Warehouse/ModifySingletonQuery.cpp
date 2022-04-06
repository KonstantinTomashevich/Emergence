#include <Galleon/SingletonContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/ModifySingletonQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using QueryImplementation = Galleon::SingletonContainer::ModifyQuery;

using CursorImplementation = QueryImplementation::Cursor;

ModifySingletonQuery::Cursor::Cursor (ModifySingletonQuery::Cursor &&_other) noexcept
{
    new (&data) CursorImplementation (std::move (block_cast<CursorImplementation> (_other.data)));
}

ModifySingletonQuery::Cursor::~Cursor () noexcept
{
    block_cast<CursorImplementation> (data).~CursorImplementation ();
}

void *ModifySingletonQuery::Cursor::operator* () const noexcept
{
    return *block_cast<CursorImplementation> (data);
}

ModifySingletonQuery::Cursor::Cursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) CursorImplementation (std::move (block_cast<CursorImplementation> (*_data)));
}

EMERGENCE_BIND_MODIFY_QUERY_COMMON_OPERATIONS (ModifySingletonQuery, QueryImplementation)

ModifySingletonQuery::Cursor ModifySingletonQuery::Execute () noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute ();
    return Cursor (reinterpret_cast<decltype (Cursor::data) *> (&cursor));
}
} // namespace Emergence::Warehouse
