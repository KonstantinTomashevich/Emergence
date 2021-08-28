#include <API/Common/Implementation/Iterator.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/KeyFieldIterator.hpp>

namespace Emergence::Warehouse
{
using KeyFieldIteratorImplementation = RecordCollection::PointRepresentation::KeyFieldIterator;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (KeyFieldIterator, KeyFieldIteratorImplementation)

StandardLayout::Field KeyFieldIterator::operator* () const noexcept
{
    return *block_cast<KeyFieldIteratorImplementation> (data);
}
} // namespace Emergence::Warehouse
