#include <cassert>

#include <Galleon/CargoDeck.hpp>
#include <Galleon/LongTermContainer.hpp>
#include <utility>

namespace Emergence::Galleon
{
void *LongTermContainer::InsertQuery::Cursor::operator ++ () noexcept
{
    return allocator.Allocate ();
}

LongTermContainer::InsertQuery::Cursor::Cursor (Handling::Handle <LongTermContainer> _container) noexcept
    : container (std::move (_container)),
      allocator (container->collection.AllocateAndInsert ())
{
    assert (container);
}

LongTermContainer::InsertQuery::Cursor LongTermContainer::InsertQuery::Execute () const noexcept
{
    return Cursor (container);
}

LongTermContainer::InsertQuery::InsertQuery (Handling::Handle <LongTermContainer> _container) noexcept
    : container (std::move (_container))
{
    assert (container);
}

LongTermContainer::FetchValueQuery::Cursor LongTermContainer::FetchValueQuery::Execute (
    RecordCollection::PointRepresentation::Point _values) noexcept
{
    return representation.ReadPoint (_values);
}

LongTermContainer::FetchValueQuery::FetchValueQuery (
    Handling::Handle <LongTermContainer> _container, RecordCollection::PointRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyValueQuery::Cursor LongTermContainer::ModifyValueQuery::Execute (
    RecordCollection::PointRepresentation::Point _values) noexcept
{
    return representation.EditPoint (_values);
}

LongTermContainer::ModifyValueQuery::ModifyValueQuery (
    Handling::Handle <LongTermContainer> _container, RecordCollection::PointRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::FetchRangeQuery::Cursor LongTermContainer::FetchRangeQuery::Execute (
    RecordCollection::LinearRepresentation::KeyFieldValue _min,
    RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept
{
    return representation.ReadInterval (_min, _max);
}

LongTermContainer::FetchRangeQuery::FetchRangeQuery (
    Handling::Handle <LongTermContainer> _container,
    RecordCollection::LinearRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyRangeQuery::Cursor LongTermContainer::ModifyRangeQuery::Execute (
    RecordCollection::LinearRepresentation::KeyFieldValue _min,
    RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept
{
    return representation.EditInterval (_min, _max);
}

LongTermContainer::ModifyRangeQuery::ModifyRangeQuery (
    Handling::Handle <LongTermContainer> _container,
    RecordCollection::LinearRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::FetchReversedRangeQuery::Cursor LongTermContainer::FetchReversedRangeQuery::Execute (
    RecordCollection::LinearRepresentation::KeyFieldValue _min,
    RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept
{
    return representation.ReadReversedInterval (_min, _max);
}

LongTermContainer::FetchReversedRangeQuery::FetchReversedRangeQuery (
    Handling::Handle <LongTermContainer> _container,
    RecordCollection::LinearRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyReversedRangeQuery::Cursor LongTermContainer::ModifyReversedRangeQuery::Execute (
    RecordCollection::LinearRepresentation::KeyFieldValue _min,
    RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept
{
    return representation.EditReversedInterval (_min, _max);
}

LongTermContainer::ModifyReversedRangeQuery::ModifyReversedRangeQuery (
    Handling::Handle <LongTermContainer> _container,
    RecordCollection::LinearRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::FetchShapeIntersectionsQuery::Cursor LongTermContainer::FetchShapeIntersectionsQuery::Execute (
    RecordCollection::VolumetricRepresentation::Shape _shape) noexcept
{
    return representation.ReadShapeIntersections (_shape);
}

LongTermContainer::FetchShapeIntersectionsQuery::FetchShapeIntersectionsQuery (
    Handling::Handle <LongTermContainer> _container,
    RecordCollection::VolumetricRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyShapeIntersectionsQuery::Cursor LongTermContainer::ModifyShapeIntersectionsQuery::Execute (
    RecordCollection::VolumetricRepresentation::Shape _shape) noexcept
{
    return representation.EditShapeIntersections (_shape);
}

LongTermContainer::ModifyShapeIntersectionsQuery::ModifyShapeIntersectionsQuery (
    Handling::Handle <LongTermContainer> _container,
    RecordCollection::VolumetricRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::FetchRayIntersectionsQuery::Cursor LongTermContainer::FetchRayIntersectionsQuery::Execute (
    RecordCollection::VolumetricRepresentation::Ray _ray, float _maxDistance) noexcept
{
    return representation.ReadRayIntersections (_ray, _maxDistance);
}

LongTermContainer::FetchRayIntersectionsQuery::FetchRayIntersectionsQuery (
    Handling::Handle <LongTermContainer> _container,
    RecordCollection::VolumetricRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

LongTermContainer::ModifyRayIntersectionsQuery::Cursor LongTermContainer::ModifyRayIntersectionsQuery::Execute (
    RecordCollection::VolumetricRepresentation::Ray _ray, float _maxDistance) noexcept
{
    return representation.EditRayIntersections (_ray, _maxDistance);
}

LongTermContainer::LongTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : ContainerBase (_deck, std::move (_typeMapping)),
      collection (typeMapping)
{
}

LongTermContainer::InsertQuery LongTermContainer::Insert () noexcept
{
    return InsertQuery (this);
}

LongTermContainer::FetchValueQuery LongTermContainer::FetchValue (
    const std::vector <StandardLayout::FieldId> &_keyFields) noexcept
{
    return {this, AcquirePointRepresentation (_keyFields)};
}

LongTermContainer::ModifyValueQuery LongTermContainer::ModifyValue
    (const std::vector <StandardLayout::FieldId> &_keyFields) noexcept
{
    return {this, AcquirePointRepresentation (_keyFields)};
}

LongTermContainer::FetchRangeQuery LongTermContainer::FetchRange (StandardLayout::FieldId _keyField) noexcept
{
    return {this, AcquireLinearRepresentation (_keyField)};
}

LongTermContainer::ModifyRangeQuery LongTermContainer::ModifyRange (StandardLayout::FieldId _keyField) noexcept
{
    return {this, AcquireLinearRepresentation (_keyField)};
}

LongTermContainer::FetchReversedRangeQuery LongTermContainer::FetchReversedRange (
    StandardLayout::FieldId _keyField) noexcept
{
    return {this, AcquireLinearRepresentation (_keyField)};
}

LongTermContainer::ModifyReversedRangeQuery LongTermContainer::ModifyReversedRange (
    StandardLayout::FieldId _keyField) noexcept
{
    return {this, AcquireLinearRepresentation (_keyField)};
}

LongTermContainer::FetchShapeIntersectionsQuery LongTermContainer::FetchShapeIntersections (
    const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
{
    return {this, AcquireVolumetricRepresentation (_dimensions)};
}

LongTermContainer::ModifyShapeIntersectionsQuery LongTermContainer::ModifyShapeIntersections (
    const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
{
    return {this, AcquireVolumetricRepresentation (_dimensions)};
}

LongTermContainer::FetchRayIntersectionsQuery LongTermContainer::FetchRayIntersections (
    const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
{
    return {this, AcquireVolumetricRepresentation (_dimensions)};
}

LongTermContainer::ModifyRayIntersectionsQuery LongTermContainer::ModifyRayIntersections (
    const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
{
    return {this, AcquireVolumetricRepresentation (_dimensions)};
}

LongTermContainer::ModifyRayIntersectionsQuery::ModifyRayIntersectionsQuery (
    Handling::Handle <LongTermContainer> _container,
    RecordCollection::VolumetricRepresentation _representation) noexcept
    : RepresentationQueryBase (std::move (_container), std::move (_representation))
{
}

RecordCollection::LinearRepresentation LongTermContainer::AcquireLinearRepresentation (
    StandardLayout::FieldId _keyField) noexcept
{
    for (auto iterator = collection.LinearRepresentationBegin ();
         iterator != collection.LinearRepresentationEnd (); ++iterator)
    {
        RecordCollection::LinearRepresentation representation = *iterator;
        if (representation.GetKeyField () == _keyField)
        {
            return representation;
        }
    }

    return collection.CreateLinearRepresentation (_keyField);
}

RecordCollection::PointRepresentation LongTermContainer::AcquirePointRepresentation (
    const std::vector <StandardLayout::FieldId> &_keyFields) noexcept
{
    for (auto iterator = collection.PointRepresentationBegin ();
         iterator != collection.PointRepresentationEnd (); ++iterator)
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

RecordCollection::VolumetricRepresentation LongTermContainer::AcquireVolumetricRepresentation (
    const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept
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

            assert (givenDimension.globalMinBorder);
            assert (givenDimension.globalMaxBorder);

            const bool minFieldMatches = representationDimension.minField.IsSame (
                typeMapping.GetField (givenDimension.minBorderField));

            const bool minValueMatches =
                minFieldMatches &&
                memcmp (representationDimension.globalMin, givenDimension.globalMinBorder,
                        representationDimension.minField.GetSize ()) == 0;

            const bool maxFieldMatches = representationDimension.maxField.IsSame (
                typeMapping.GetField (givenDimension.maxBorderField));

            const bool maxValueMatches =
                maxFieldMatches &&
                memcmp (representationDimension.globalMax, givenDimension.globalMaxBorder,
                        representationDimension.maxField.GetSize ()) == 0;

            if (!minFieldMatches || !maxFieldMatches || !minValueMatches || !maxValueMatches)
            {
                match = false;
                break;
            }
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

LongTermContainer::~LongTermContainer () noexcept
{
    assert (deck);
    deck->DetachContainer (this);
}
} // namespace Emergence::Galleon