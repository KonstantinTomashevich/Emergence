#include <API/Common/Implementation/Iterator.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/Dimension.hpp>

namespace Emergence::Warehouse
{
using DimensionIteratorImplementation = RecordCollection::VolumetricRepresentation::DimensionIterator;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (DimensionIterator, DimensionIteratorImplementation)

Dimension DimensionIterator::operator* () const noexcept
{
    auto dimension = *block_cast<DimensionIteratorImplementation> (data);
    return {dimension.globalMin, dimension.minField, dimension.globalMax, dimension.maxField};
}
} // namespace Emergence::Warehouse
