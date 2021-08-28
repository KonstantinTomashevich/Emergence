#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

#include <StandardLayout/Field.hpp>

namespace Emergence::Warehouse
{
/// \brief Provides iteration over key fields of queries, that have multiple key fields.
class KeyFieldIterator final
{
public:
    EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (KeyFieldIterator, StandardLayout::Field);

private:
    /// These prepared queries construct dimension iterators.
    friend class FetchValueQuery;

    friend class ModifyValueQuery;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

    explicit KeyFieldIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};
} // namespace Emergence::Warehouse