#include <cstring>

#include <Galleon/CargoDeck.hpp>
#include <Galleon/LongTermContainer.hpp>

namespace Emergence::Galleon
{
void *LongTermContainer::InsertQuery::Cursor::operator++ () noexcept
{
    return allocator.Allocate ();
}

LongTermContainer::InsertQuery::Cursor::Cursor (Handling::Handle<LongTermContainer> _container) noexcept
    : container (std::move (_container)),
      allocator (container->collection.AllocateAndInsert ())
{
    EMERGENCE_ASSERT (container);
}

LongTermContainer::InsertQuery::Cursor LongTermContainer::InsertQuery::Execute () const noexcept
{
    return Cursor (container);
}

Handling::Handle<LongTermContainer> LongTermContainer::InsertQuery::GetContainer () const noexcept
{
    EMERGENCE_ASSERT (container);
    return container;
}

LongTermContainer::InsertQuery::InsertQuery (Handling::Handle<LongTermContainer> _container) noexcept
    : container (std::move (_container))
{
    EMERGENCE_ASSERT (container);
}

LongTermContainer::FetchValueQuery::Cursor LongTermContainer::FetchValueQuery::Execute (
    RecordCollection::PointRepresentation::Point _values) noexcept
{
    return representation.ReadPoint (_values);
}

RecordCollection::PointRepresentation::KeyFieldIterator LongTermContainer::FetchValueQuery::KeyFieldBegin ()
    const noexcept
{
    return representation.KeyFieldBegin ();
}

RecordCollection::PointRepresentation::KeyFieldIterator LongTermContainer::FetchValueQuery::KeyFieldEnd ()
    const noexcept
{
    return representation.KeyFieldEnd ();
}

LongTermContainer::FetchValueQuery::FetchValueQuery (Handling::Handle<LongTermContainer> _container,
                                                     RecordCollection::PointRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyValueQuery::Cursor LongTermContainer::ModifyValueQuery::Execute (
    RecordCollection::PointRepresentation::Point _values) noexcept
{
    return representation.EditPoint (_values);
}

RecordCollection::PointRepresentation::KeyFieldIterator LongTermContainer::ModifyValueQuery::KeyFieldBegin ()
    const noexcept
{
    return representation.KeyFieldBegin ();
}

RecordCollection::PointRepresentation::KeyFieldIterator LongTermContainer::ModifyValueQuery::KeyFieldEnd ()
    const noexcept
{
    return representation.KeyFieldEnd ();
}

LongTermContainer::ModifyValueQuery::ModifyValueQuery (Handling::Handle<LongTermContainer> _container,
                                                       RecordCollection::PointRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::FetchAscendingRangeQuery::Cursor LongTermContainer::FetchAscendingRangeQuery::Execute (
    RecordCollection::LinearRepresentation::KeyFieldValue _min,
    RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept
{
    return representation.ReadAscendingInterval (_min, _max);
}

StandardLayout::Field LongTermContainer::FetchAscendingRangeQuery::GetKeyField () const noexcept
{
    return representation.GetKeyField ();
}

LongTermContainer::FetchAscendingRangeQuery::FetchAscendingRangeQuery (
    Handling::Handle<LongTermContainer> _container, RecordCollection::LinearRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyAscendingRangeQuery::Cursor LongTermContainer::ModifyAscendingRangeQuery::Execute (
    RecordCollection::LinearRepresentation::KeyFieldValue _min,
    RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept
{
    return representation.EditAscendingInterval (_min, _max);
}

StandardLayout::Field LongTermContainer::ModifyAscendingRangeQuery::GetKeyField () const noexcept
{
    return representation.GetKeyField ();
}

LongTermContainer::ModifyAscendingRangeQuery::ModifyAscendingRangeQuery (
    Handling::Handle<LongTermContainer> _container, RecordCollection::LinearRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::FetchDescendingRangeQuery::Cursor LongTermContainer::FetchDescendingRangeQuery::Execute (
    RecordCollection::LinearRepresentation::KeyFieldValue _min,
    RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept
{
    return representation.ReadDescendingInterval (_min, _max);
}

StandardLayout::Field LongTermContainer::FetchDescendingRangeQuery::GetKeyField () const noexcept
{
    return representation.GetKeyField ();
}

LongTermContainer::FetchDescendingRangeQuery::FetchDescendingRangeQuery (
    Handling::Handle<LongTermContainer> _container, RecordCollection::LinearRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyDescendingRangeQuery::Cursor LongTermContainer::ModifyDescendingRangeQuery::Execute (
    RecordCollection::LinearRepresentation::KeyFieldValue _min,
    RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept
{
    return representation.EditDescendingInterval (_min, _max);
}

StandardLayout::Field LongTermContainer::ModifyDescendingRangeQuery::GetKeyField () const noexcept
{
    return representation.GetKeyField ();
}

LongTermContainer::ModifyDescendingRangeQuery::ModifyDescendingRangeQuery (
    Handling::Handle<LongTermContainer> _container, RecordCollection::LinearRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::FetchSignalQuery::Cursor LongTermContainer::FetchSignalQuery::Execute () noexcept
{
    return representation.ReadSignaled ();
}

StandardLayout::Field LongTermContainer::FetchSignalQuery::GetKeyField () const noexcept
{
    return representation.GetKeyField ();
}

bool LongTermContainer::FetchSignalQuery::IsSignaledValue (const std::array<uint8_t, sizeof (uint64_t)> &_value) const
{
    return representation.IsSignaledValue (_value);
}

std::array<uint8_t, sizeof (uint64_t)> LongTermContainer::FetchSignalQuery::GetSignaledValue () const noexcept
{
    return representation.GetSignaledValue ();
}

LongTermContainer::FetchSignalQuery::FetchSignalQuery (Handling::Handle<LongTermContainer> _container,
                                                       RecordCollection::SignalRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifySignalQuery::Cursor LongTermContainer::ModifySignalQuery::Execute () noexcept
{
    return representation.EditSignaled ();
}

StandardLayout::Field LongTermContainer::ModifySignalQuery::GetKeyField () const noexcept
{
    return representation.GetKeyField ();
}

bool LongTermContainer::ModifySignalQuery::IsSignaledValue (const std::array<uint8_t, sizeof (uint64_t)> &_value) const
{
    return representation.IsSignaledValue (_value);
}

std::array<uint8_t, sizeof (uint64_t)> LongTermContainer::ModifySignalQuery::GetSignaledValue () const noexcept
{
    return representation.GetSignaledValue ();
}

LongTermContainer::ModifySignalQuery::ModifySignalQuery (
    Handling::Handle<LongTermContainer> _container, RecordCollection::SignalRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::FetchShapeIntersectionQuery::Cursor LongTermContainer::FetchShapeIntersectionQuery::Execute (
    RecordCollection::VolumetricRepresentation::Shape _shape) noexcept
{
    return representation.ReadShapeIntersections (_shape);
}

RecordCollection::VolumetricRepresentation::DimensionIterator
LongTermContainer::FetchShapeIntersectionQuery::DimensionBegin () const noexcept
{
    return representation.DimensionBegin ();
}

RecordCollection::VolumetricRepresentation::DimensionIterator
LongTermContainer::FetchShapeIntersectionQuery::DimensionEnd () const noexcept
{
    return representation.DimensionEnd ();
}

LongTermContainer::FetchShapeIntersectionQuery::FetchShapeIntersectionQuery (
    Handling::Handle<LongTermContainer> _container, RecordCollection::VolumetricRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyShapeIntersectionQuery::Cursor LongTermContainer::ModifyShapeIntersectionQuery::Execute (
    RecordCollection::VolumetricRepresentation::Shape _shape) noexcept
{
    return representation.EditShapeIntersections (_shape);
}

RecordCollection::VolumetricRepresentation::DimensionIterator
LongTermContainer::ModifyShapeIntersectionQuery::DimensionBegin () const noexcept
{
    return representation.DimensionBegin ();
}

RecordCollection::VolumetricRepresentation::DimensionIterator
LongTermContainer::ModifyShapeIntersectionQuery::DimensionEnd () const noexcept
{
    return representation.DimensionEnd ();
}

LongTermContainer::ModifyShapeIntersectionQuery::ModifyShapeIntersectionQuery (
    Handling::Handle<LongTermContainer> _container, RecordCollection::VolumetricRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::FetchRayIntersectionQuery::Cursor LongTermContainer::FetchRayIntersectionQuery::Execute (
    RecordCollection::VolumetricRepresentation::Ray _ray, float _maxDistance) noexcept
{
    return representation.ReadRayIntersections (_ray, _maxDistance);
}

RecordCollection::VolumetricRepresentation::DimensionIterator
LongTermContainer::FetchRayIntersectionQuery::DimensionBegin () const noexcept
{
    return representation.DimensionBegin ();
}

RecordCollection::VolumetricRepresentation::DimensionIterator
LongTermContainer::FetchRayIntersectionQuery::DimensionEnd () const noexcept
{
    return representation.DimensionEnd ();
}

LongTermContainer::FetchRayIntersectionQuery::FetchRayIntersectionQuery (
    Handling::Handle<LongTermContainer> _container, RecordCollection::VolumetricRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyRayIntersectionQuery::Cursor LongTermContainer::ModifyRayIntersectionQuery::Execute (
    RecordCollection::VolumetricRepresentation::Ray _ray, float _maxDistance) noexcept
{
    return representation.EditRayIntersections (_ray, _maxDistance);
}

RecordCollection::VolumetricRepresentation::DimensionIterator
LongTermContainer::ModifyRayIntersectionQuery::DimensionBegin () const noexcept
{
    return representation.DimensionBegin ();
}

RecordCollection::VolumetricRepresentation::DimensionIterator
LongTermContainer::ModifyRayIntersectionQuery::DimensionEnd () const noexcept
{
    return representation.DimensionEnd ();
}

LongTermContainer::ModifyRayIntersectionQuery::ModifyRayIntersectionQuery (
    Handling::Handle<LongTermContainer> _container, RecordCollection::VolumetricRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::InsertQuery LongTermContainer::Insert () noexcept
{
    return InsertQuery (this);
}

LongTermContainer::FetchValueQuery LongTermContainer::FetchValue (
    const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    return {this, AcquirePointRepresentation (_keyFields)};
}

LongTermContainer::ModifyValueQuery LongTermContainer::ModifyValue (
    const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    return {this, AcquirePointRepresentation (_keyFields)};
}

LongTermContainer::FetchAscendingRangeQuery LongTermContainer::FetchAscendingRange (
    StandardLayout::FieldId _keyField) noexcept
{
    return {this, AcquireLinearRepresentation (_keyField)};
}

LongTermContainer::ModifyAscendingRangeQuery LongTermContainer::ModifyAscendingRange (
    StandardLayout::FieldId _keyField) noexcept
{
    return {this, AcquireLinearRepresentation (_keyField)};
}

LongTermContainer::FetchDescendingRangeQuery LongTermContainer::FetchDescendingRange (
    StandardLayout::FieldId _keyField) noexcept
{
    return {this, AcquireLinearRepresentation (_keyField)};
}

LongTermContainer::ModifyDescendingRangeQuery LongTermContainer::ModifyDescendingRange (
    StandardLayout::FieldId _keyField) noexcept
{
    return {this, AcquireLinearRepresentation (_keyField)};
}

LongTermContainer::FetchSignalQuery LongTermContainer::FetchSignal (
    StandardLayout::FieldId _keyField, const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    return {this, AcquireSignalRepresentation (_keyField, _signaledValue)};
}

LongTermContainer::ModifySignalQuery LongTermContainer::ModifySignal (
    StandardLayout::FieldId _keyField, const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    return {this, AcquireSignalRepresentation (_keyField, _signaledValue)};
}

LongTermContainer::FetchShapeIntersectionQuery LongTermContainer::FetchShapeIntersection (
    const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
{
    return {this, AcquireVolumetricRepresentation (_dimensions)};
}

LongTermContainer::ModifyShapeIntersectionQuery LongTermContainer::ModifyShapeIntersection (
    const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
{
    return {this, AcquireVolumetricRepresentation (_dimensions)};
}

LongTermContainer::FetchRayIntersectionQuery LongTermContainer::FetchRayIntersection (
    const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
{
    return {this, AcquireVolumetricRepresentation (_dimensions)};
}

LongTermContainer::ModifyRayIntersectionQuery LongTermContainer::ModifyRayIntersection (
    const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
{
    return {this, AcquireVolumetricRepresentation (_dimensions)};
}

void LongTermContainer::LastReferenceUnregistered () noexcept
{
    EMERGENCE_ASSERT (deck);
    deck->DetachContainer (this);
}

void LongTermContainer::SetUnsafeFetchAllowed (bool _allowed) noexcept
{
    collection.SetUnsafeReadAllowed (_allowed);
}

static RecordCollection::Collection ConstructInsideGroup (StandardLayout::Mapping _typeMapping)
{
    auto placeholder = Memory::Profiler::AllocationGroup {Memory::UniqueString {_typeMapping.GetName ()}}.PlaceOnTop ();
    return RecordCollection::Collection {std::move (_typeMapping)};
}

LongTermContainer::LongTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping)),
      collection (ConstructInsideGroup (typeMapping))
{
}

RecordCollection::LinearRepresentation LongTermContainer::AcquireLinearRepresentation (
    StandardLayout::FieldId _keyField) noexcept
{
    for (auto iterator = collection.LinearRepresentationBegin (); iterator != collection.LinearRepresentationEnd ();
         ++iterator)
    {
        RecordCollection::LinearRepresentation representation = *iterator;
        if (representation.GetKeyField ().IsSame (typeMapping.GetField (_keyField)))
        {
            return representation;
        }
    }

    return collection.CreateLinearRepresentation (_keyField);
}

RecordCollection::PointRepresentation LongTermContainer::AcquirePointRepresentation (
    const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    for (auto iterator = collection.PointRepresentationBegin (); iterator != collection.PointRepresentationEnd ();
         ++iterator)
    {
        RecordCollection::PointRepresentation representation = *iterator;
        auto representationKeyFieldIterator = representation.KeyFieldBegin ();
        auto givenKeyFieldIterator = _keyFields.begin ();
        bool match = true;

        while (representationKeyFieldIterator != representation.KeyFieldEnd () &&
               givenKeyFieldIterator != _keyFields.end ())
        {
            if (!(*representationKeyFieldIterator).IsSame (typeMapping.GetField (*givenKeyFieldIterator)))
            {
                match = false;
                break;
            }

            ++representationKeyFieldIterator;
            ++givenKeyFieldIterator;
        }

        match &= representationKeyFieldIterator == representation.KeyFieldEnd () &&
                 givenKeyFieldIterator == _keyFields.end ();

        if (match)
        {
            return representation;
        }
    }

    return collection.CreatePointRepresentation (_keyFields);
}

RecordCollection::SignalRepresentation LongTermContainer::AcquireSignalRepresentation (
    StandardLayout::FieldId _keyField, const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    for (auto iterator = collection.SignalRepresentationBegin (); iterator != collection.SignalRepresentationEnd ();
         ++iterator)
    {
        RecordCollection::SignalRepresentation representation = *iterator;
        if (representation.GetKeyField ().IsSame (typeMapping.GetField (_keyField)) &&
            representation.IsSignaledValue (_signaledValue))
        {
            return representation;
        }
    }

    return collection.CreateSignalRepresentation (_keyField, _signaledValue);
}

RecordCollection::VolumetricRepresentation LongTermContainer::AcquireVolumetricRepresentation (
    const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
{
    for (auto iterator = collection.VolumetricRepresentationBegin ();
         iterator != collection.VolumetricRepresentationEnd (); ++iterator)
    {
        RecordCollection::VolumetricRepresentation representation = *iterator;
        auto representationDimensionIterator = representation.DimensionBegin ();
        auto givenDimensionIterator = _dimensions.begin ();
        bool match = true;

        while (representationDimensionIterator != representation.DimensionEnd () &&
               givenDimensionIterator != _dimensions.end ())
        {
            const auto representationDimension = *representationDimensionIterator;
            const auto &givenDimension = *givenDimensionIterator;

            EMERGENCE_ASSERT (givenDimension.globalMinBorder);
            EMERGENCE_ASSERT (givenDimension.globalMaxBorder);

            const bool minFieldMatches =
                representationDimension.minField.IsSame (typeMapping.GetField (givenDimension.minBorderField));

            const bool minValueMatches =
                minFieldMatches && memcmp (representationDimension.globalMin, givenDimension.globalMinBorder,
                                           representationDimension.minField.GetSize ()) == 0;

            const bool maxFieldMatches =
                representationDimension.maxField.IsSame (typeMapping.GetField (givenDimension.maxBorderField));

            const bool maxValueMatches =
                maxFieldMatches && memcmp (representationDimension.globalMax, givenDimension.globalMaxBorder,
                                           representationDimension.maxField.GetSize ()) == 0;

            if (!minFieldMatches || !maxFieldMatches || !minValueMatches || !maxValueMatches)
            {
                match = false;
                break;
            }

            ++representationDimensionIterator;
            ++givenDimensionIterator;
        }

        match &= representationDimensionIterator == representation.DimensionEnd () &&
                 givenDimensionIterator == _dimensions.end ();

        if (match)
        {
            return representation;
        }
    }

    return collection.CreateVolumetricRepresentation (_dimensions);
}
} // namespace Emergence::Galleon
