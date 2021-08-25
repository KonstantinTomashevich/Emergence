#include <cassert>

#include <Galleon/CargoDeck.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/Registry.hpp>

namespace Emergence::Warehouse
{
static Handling::Handle <Galleon::SingletonContainer> UseSingletonContainer (
    Galleon::CargoDeck &_deck, const StandardLayout::Mapping &_typeMapping)
{
    assert (!_deck.IsShortTermContainerAllocated (_typeMapping));
    assert (!_deck.IsLongTermContainerAllocated (_typeMapping));
    return _deck.AcquireSingletonContainer (_typeMapping);
}

static Handling::Handle <Galleon::ShortTermContainer> UseShortTermContainer (
    Galleon::CargoDeck &_deck, const StandardLayout::Mapping &_typeMapping)
{
    assert (!_deck.IsSingletonContainerAllocated (_typeMapping));
    assert (!_deck.IsLongTermContainerAllocated (_typeMapping));
    return _deck.AcquireShortTermContainer (_typeMapping);
}

static Handling::Handle <Galleon::LongTermContainer> UseLongTermContainer (
    Galleon::CargoDeck &_deck, const StandardLayout::Mapping &_typeMapping)
{
    assert (!_deck.IsSingletonContainerAllocated (_typeMapping));
    assert (!_deck.IsShortTermContainerAllocated (_typeMapping));
    return _deck.AcquireLongTermContainer (_typeMapping);
}

std::vector <RecordCollection::Collection::DimensionDescriptor> ConvertDimensions (
    const StandardLayout::Mapping &_typeMapping, const std::vector <Dimension> &_dimensions)
{
    // Query preparation is called rarely, therefore vector allocation is ok.
    std::vector <RecordCollection::Collection::DimensionDescriptor> result;
    result.reserve (_dimensions.size ());

    for (const Dimension &dimension : _dimensions)
    {
        result.emplace_back (
            RecordCollection::Collection::DimensionDescriptor
                {
                    dimension.globalMinBorder, _typeMapping.GetFieldId (dimension.minBorderField),
                    dimension.globalMaxBorder, _typeMapping.GetFieldId (dimension.maxBorderField)
                });
    }

    return result;
}

Registry::Registry () noexcept
    : handle (new Galleon::CargoDeck ())
{
}

Registry::Registry (Registry &&_other) noexcept
    : handle (_other.handle)
{
    assert (handle);
    _other.handle = nullptr;
}

Registry::~Registry () noexcept
{
    delete static_cast <Galleon::CargoDeck *> (handle);
}

FetchSingletonQuery Registry::FetchSingleton (const StandardLayout::Mapping &_typeMapping) noexcept
{
    assert (handle);
    auto query = UseSingletonContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping)->Fetch ();
    return FetchSingletonQuery (reinterpret_cast <decltype (FetchSingletonQuery::data) *> (&query));
}

ModifySingletonQuery Registry::ModifySingleton (const StandardLayout::Mapping &_typeMapping) noexcept
{
    assert (handle);
    auto query = UseSingletonContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping)->Modify ();
    return ModifySingletonQuery (reinterpret_cast <decltype (ModifySingletonQuery::data) *> (&query));
}

InsertShortTermQuery Registry::InsertShortTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    assert (handle);
    auto query = UseShortTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping)->Insert ();
    return InsertShortTermQuery (reinterpret_cast <decltype (InsertShortTermQuery::data) *> (&query));
}

FetchSequenceQuery Registry::FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    assert (handle);
    auto query = UseShortTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping)->Fetch ();
    return FetchSequenceQuery (reinterpret_cast <decltype (FetchSequenceQuery::data) *> (&query));
}

ModifySequenceQuery Registry::ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept
{
    assert (handle);
    auto query = UseShortTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping)->Modify ();
    return ModifySequenceQuery (reinterpret_cast <decltype (ModifySequenceQuery::data) *> (&query));
}

InsertLongTermQuery Registry::InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept
{
    assert (handle);
    auto query = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping)->Insert ();
    return InsertLongTermQuery (reinterpret_cast <decltype (InsertLongTermQuery::data) *> (&query));
}

FetchValueQuery Registry::FetchValue (const StandardLayout::Mapping &_typeMapping,
                                      const std::vector <StandardLayout::FieldId> &_keyFields) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->FetchValue (_keyFields);
    return FetchValueQuery (reinterpret_cast <decltype (FetchValueQuery::data) *> (&query));
}

ModifyValueQuery Registry::ModifyValue (const StandardLayout::Mapping &_typeMapping,
                                        const std::vector <StandardLayout::FieldId> &_keyFields) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->ModifyValue (_keyFields);
    return ModifyValueQuery (reinterpret_cast <decltype (ModifyValueQuery::data) *> (&query));
}

FetchAscendingRangeQuery Registry::FetchAscendingRange (
    const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->FetchAscendingRange (_keyField);
    return FetchAscendingRangeQuery (reinterpret_cast <decltype (FetchAscendingRangeQuery::data) *> (&query));
}

ModifyAscendingRangeQuery Registry::ModifyAscendingRange (
    const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->ModifyAscendingRange (_keyField);
    return ModifyAscendingRangeQuery (reinterpret_cast <decltype (ModifyAscendingRangeQuery::data) *> (&query));
}

FetchDescendingRangeQuery Registry::FetchDescendingRange (
    const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->FetchDescendingRange (_keyField);
    return FetchDescendingRangeQuery (reinterpret_cast <decltype (FetchDescendingRangeQuery::data) *> (&query));
}

ModifyDescendingRangeQuery Registry::ModifyDescendingRange (
    const StandardLayout::Mapping &_typeMapping, StandardLayout::FieldId _keyField) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->ModifyDescendingRange (_keyField);
    return ModifyDescendingRangeQuery (reinterpret_cast <decltype (ModifyDescendingRangeQuery::data) *> (&query));
}

FetchShapeIntersectionQuery Registry::FetchShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const std::vector <Dimension> &_dimensions) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->FetchShapeIntersection (ConvertDimensions (_typeMapping, _dimensions));
    return FetchShapeIntersectionQuery (reinterpret_cast <decltype (FetchShapeIntersectionQuery::data) *> (&query));
}

ModifyShapeIntersectionQuery Registry::ModifyShapeIntersection (
    const StandardLayout::Mapping &_typeMapping, const std::vector <Dimension> &_dimensions) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->ModifyShapeIntersection (ConvertDimensions (_typeMapping, _dimensions));
    return ModifyShapeIntersectionQuery (reinterpret_cast <decltype (ModifyShapeIntersectionQuery::data) *> (&query));
}

FetchRayIntersectionQuery Registry::FetchRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const std::vector <Dimension> &_dimensions) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->FetchRayIntersection (ConvertDimensions (_typeMapping, _dimensions));
    return FetchRayIntersectionQuery (reinterpret_cast <decltype (FetchRayIntersectionQuery::data) *> (&query));
}

ModifyRayIntersectionQuery Registry::ModifyRayIntersection (
    const StandardLayout::Mapping &_typeMapping, const std::vector <Dimension> &_dimensions) noexcept
{
    assert (handle);
    auto container = UseLongTermContainer (*static_cast <Galleon::CargoDeck *> (handle), _typeMapping);
    auto query = container->ModifyRayIntersection (ConvertDimensions (_typeMapping, _dimensions));
    return ModifyRayIntersectionQuery (reinterpret_cast <decltype (ModifyRayIntersectionQuery::data) *> (&query));
}

Registry &Registry::operator = (Registry &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Registry ();
        new (this) Registry (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Warehouse