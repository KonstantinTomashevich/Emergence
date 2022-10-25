#include <Galleon/CargoDeck.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/Registry.hpp>
#include <Warehouse/VisualizationDriver.hpp>

namespace Emergence::Warehouse
{
static Handling::Handle<Galleon::SingletonContainer> UseSingletonContainer (Galleon::CargoDeck &_deck,
                                                                            const StandardLayout::Mapping &_typeMapping)
{
    EMERGENCE_ASSERT (!_deck.IsShortTermContainerAllocated (_typeMapping));
    EMERGENCE_ASSERT (!_deck.IsLongTermContainerAllocated (_typeMapping));
    return _deck.AcquireSingletonContainer (_typeMapping);
}

static Handling::Handle<Galleon::ShortTermContainer> UseShortTermContainer (Galleon::CargoDeck &_deck,
                                                                            const StandardLayout::Mapping &_typeMapping)
{
    EMERGENCE_ASSERT (!_deck.IsSingletonContainerAllocated (_typeMapping));
    EMERGENCE_ASSERT (!_deck.IsLongTermContainerAllocated (_typeMapping));
    return _deck.AcquireShortTermContainer (_typeMapping);
}

static Handling::Handle<Galleon::LongTermContainer> UseLongTermContainer (Galleon::CargoDeck &_deck,
                                                                          const StandardLayout::Mapping &_typeMapping)
{
    EMERGENCE_ASSERT (!_deck.IsSingletonContainerAllocated (_typeMapping));
    EMERGENCE_ASSERT (!_deck.IsShortTermContainerAllocated (_typeMapping));
    return _deck.AcquireLongTermContainer (_typeMapping);
}

Container::Vector<RecordCollection::Collection::DimensionDescriptor> ConvertDimensions (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Dimension> &_dimensions)
{
    // Query preparation is called rarely, therefore vector allocation is ok.
    Container::Vector<RecordCollection::Collection::DimensionDescriptor> result;
    result.reserve (_dimensions.size ());

    for (const Dimension &dimension : _dimensions)
    {
        result.emplace_back (RecordCollection::Collection::DimensionDescriptor {
            dimension.globalMinBorder, _typeMapping.GetFieldId (dimension.minBorderField), dimension.globalMaxBorder,
            _typeMapping.GetFieldId (dimension.maxBorderField)});
    }

    return result;
}

struct InternalData final
{
    Memory::Heap heap {Memory::Profiler::AllocationGroup (Memory::UniqueString {"Registry"})};
    Galleon::CargoDeck *deck = nullptr;
};

Registry::Registry (Memory::UniqueString _name) noexcept
{
    auto &internal = *new (&data) InternalData ();
    auto placeholder = internal.heap.GetAllocationGroup ().PlaceOnTop ();
    internal.deck = new (internal.heap.Acquire (sizeof (Galleon::CargoDeck), alignof (Galleon::CargoDeck)))
        Galleon::CargoDeck (_name);
}

Registry::Registry (Registry &&_other) noexcept
{
    auto &internal = *new (&data) InternalData ();
    internal.deck = block_cast<InternalData> (_other.data).deck;
    EMERGENCE_ASSERT (internal.deck);
    block_cast<InternalData> (_other.data).deck = nullptr;
}

Registry::~Registry () noexcept
{
    auto &internal = block_cast<InternalData> (data);
    if (internal.deck)
    {
        internal.deck->~CargoDeck ();
        internal.heap.Release (internal.deck, sizeof (Galleon::CargoDeck));
    }

    internal.~InternalData ();
}

FetchSingletonQuery Registry::FetchSingleton (const StandardLayout::Mapping &_typeMapping) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto query = UseSingletonContainer (*internal.deck, _typeMapping)->Fetch ();
    return FetchSingletonQuery (array_cast (query));
}

ModifySingletonQuery Registry::ModifySingleton (const StandardLayout::Mapping &_typeMapping) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto query = UseSingletonContainer (*internal.deck, _typeMapping)->Modify ();
    return ModifySingletonQuery (array_cast (query));
}

InsertShortTermQuery Registry::InsertShortTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto query = UseShortTermContainer (*internal.deck, _typeMapping)->Insert ();
    return InsertShortTermQuery (array_cast (query));
}

FetchSequenceQuery Registry::FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto query = UseShortTermContainer (*internal.deck, _typeMapping)->Fetch ();
    return FetchSequenceQuery (array_cast (query));
}

ModifySequenceQuery Registry::ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto query = UseShortTermContainer (*internal.deck, _typeMapping)->Modify ();
    return ModifySequenceQuery (array_cast (query));
}

InsertLongTermQuery Registry::InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto query = UseLongTermContainer (*internal.deck, _typeMapping)->Insert ();
    return InsertLongTermQuery (array_cast (query));
}

FetchValueQuery Registry::FetchValue (const StandardLayout::Mapping &_typeMapping,
                                      const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->FetchValue (_keyFields);
    return FetchValueQuery (array_cast (query));
}

ModifyValueQuery Registry::ModifyValue (const StandardLayout::Mapping &_typeMapping,
                                        const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->ModifyValue (_keyFields);
    return ModifyValueQuery (array_cast (query));
}

FetchAscendingRangeQuery Registry::FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                        StandardLayout::FieldId _keyField) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->FetchAscendingRange (_keyField);
    return FetchAscendingRangeQuery (array_cast (query));
}

ModifyAscendingRangeQuery Registry::ModifyAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                          StandardLayout::FieldId _keyField) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->ModifyAscendingRange (_keyField);
    return ModifyAscendingRangeQuery (array_cast (query));
}

FetchDescendingRangeQuery Registry::FetchDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                          StandardLayout::FieldId _keyField) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->FetchDescendingRange (_keyField);
    return FetchDescendingRangeQuery (array_cast (query));
}

ModifyDescendingRangeQuery Registry::ModifyDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                            StandardLayout::FieldId _keyField) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->ModifyDescendingRange (_keyField);
    return ModifyDescendingRangeQuery (array_cast (query));
}

FetchSignalQuery Registry::FetchSignal (const StandardLayout::Mapping &_typeMapping,
                                        StandardLayout::FieldId _keyField,
                                        const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->FetchSignal (_keyField, _signaledValue);
    return FetchSignalQuery (array_cast (query));
}

ModifySignalQuery Registry::ModifySignal (const StandardLayout::Mapping &_typeMapping,
                                          StandardLayout::FieldId _keyField,
                                          const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->ModifySignal (_keyField, _signaledValue);
    return ModifySignalQuery (array_cast (query));
}

FetchShapeIntersectionQuery Registry::FetchShapeIntersection (const StandardLayout::Mapping &_typeMapping,
                                                              const Container::Vector<Dimension> &_dimensions) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->FetchShapeIntersection (ConvertDimensions (_typeMapping, _dimensions));
    return FetchShapeIntersectionQuery (array_cast (query));
}

ModifyShapeIntersectionQuery Registry::ModifyShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const Container::Vector<Dimension> &_dimensions) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->ModifyShapeIntersection (ConvertDimensions (_typeMapping, _dimensions));
    return ModifyShapeIntersectionQuery (array_cast (query));
}

FetchRayIntersectionQuery Registry::FetchRayIntersection (const StandardLayout::Mapping &_typeMapping,
                                                          const Container::Vector<Dimension> &_dimensions) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->FetchRayIntersection (ConvertDimensions (_typeMapping, _dimensions));
    return FetchRayIntersectionQuery (array_cast (query));
}

ModifyRayIntersectionQuery Registry::ModifyRayIntersection (const StandardLayout::Mapping &_typeMapping,
                                                            const Container::Vector<Dimension> &_dimensions) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    auto container = UseLongTermContainer (*internal.deck, _typeMapping);
    auto query = container->ModifyRayIntersection (ConvertDimensions (_typeMapping, _dimensions));
    return ModifyRayIntersectionQuery (array_cast (query));
}

Memory::UniqueString Registry::GetName () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    return internal.deck->GetName ();
}

void Registry::AddCustomVisualization (VisualGraph::Graph &_graph) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.deck);
    Galleon::VisualizationDriver::PostProcess (_graph, *internal.deck);
}

Registry &Registry::operator= (Registry &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Registry ();
        new (this) Registry (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Warehouse
