#include <API/Common/Implementation/Iterator.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/Dimension.hpp>

namespace Emergence
{
namespace Warehouse
{
using DimensionIteratorImplementation = RecordCollection::VolumetricRepresentation::DimensionIterator;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (DimensionIterator, DimensionIteratorImplementation)

Dimension DimensionIterator::operator* () const noexcept
{
    auto dimension = *block_cast<DimensionIteratorImplementation> (data);
    return {dimension.globalMin, dimension.minField, dimension.globalMax, dimension.maxField};
}
} // namespace Warehouse

namespace Memory
{
const UniqueString DefaultAllocationGroup<Warehouse::Dimension>::ID {"DimensionConfiguration"};
} // namespace Memory
} // namespace Emergence
