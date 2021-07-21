#include <optional>
#include <sstream>
#include <unordered_map>

#include <RecordCollection/Collection.hpp>
#include <RecordCollection/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::RecordCollection::Test
{
using RepresentationReference = std::variant <
    LinearRepresentation,
    PointRepresentation,
    VolumetricRepresentation>;

using Cursor = std::variant <
    LinearRepresentation::ReadCursor,
    LinearRepresentation::EditCursor,
    LinearRepresentation::ReversedReadCursor,
    LinearRepresentation::ReversedEditCursor,
    PointRepresentation::ReadCursor,
    PointRepresentation::EditCursor,
    VolumetricRepresentation::ShapeIntersectionReadCursor,
    VolumetricRepresentation::ShapeIntersectionEditCursor,
    VolumetricRepresentation::RayIntersectionReadCursor,
    VolumetricRepresentation::RayIntersectionEditCursor>;

template <typename CursorType>
constexpr bool isEditCursor =
    std::is_same_v <CursorType, LinearRepresentation::EditCursor> ||
    std::is_same_v <CursorType, LinearRepresentation::ReversedEditCursor> ||
    std::is_same_v <CursorType, PointRepresentation::EditCursor> ||
    std::is_same_v <CursorType, VolumetricRepresentation::ShapeIntersectionEditCursor> ||
    std::is_same_v <CursorType, VolumetricRepresentation::RayIntersectionEditCursor>;

struct ExecutionContext
{
    explicit ExecutionContext (const StandardLayout::Mapping &_typeMapping);

    void ExecuteTask (const CreatePointRepresentation &_task);

    void ExecuteTask (const CreateLinearRepresentation &_task);

    void ExecuteTask (const CreateVolumetricRepresentation &_task);

    void ExecuteTask (const CopyRepresentationReference &_task);

    void ExecuteTask (const RemoveRepresentationReference &_task);

    void ExecuteTask (const CheckIsSourceBusy &_task);

    void ExecuteTask (const DropRepresentation &_task);

    void ExecuteTask (const OpenAllocator &);

    void ExecuteTask (const AllocateAndInit &_task);

    void ExecuteTask (const CloseAllocator &);

    void ExecuteTask (const QueryValueToRead &_task);

    void ExecuteTask (const QueryValueToEdit &_task);

    void ExecuteTask (const QueryRangeToRead &_task);

    void ExecuteTask (const QueryRangeToEdit &_task);

    void ExecuteTask (const QueryReversedRangeToRead &_task);

    void ExecuteTask (const QueryReversedRangeToEdit &_task);

    void ExecuteTask (const QueryShapeIntersectionToRead &_task);

    void ExecuteTask (const QueryShapeIntersectionToEdit &_task);

    void ExecuteTask (const QueryRayIntersectionToRead &_task);

    void ExecuteTask (const QueryRayIntersectionToEdit &_task);

    void ExecuteTask (const CursorCheck &_task);

    void ExecuteTask (const CursorCheckAllOrdered &_task);

    void ExecuteTask (const CursorCheckAllUnordered &_task);

    void ExecuteTask (const CursorEdit &_task);

    void ExecuteTask (const CursorIncrement &_task);

    void ExecuteTask (const CursorDeleteObject &_task);

    void ExecuteTask (const CursorCopy &_task);

    void ExecuteTask (const CursorMove &_task);

    void ExecuteTask (const CursorClose &_task);

    const RepresentationReference &PrepareForLookup (const QueryBase &_task) const;

    void IterateOverRepresentations () const;

    std::string RecordToString (const void *_record) const;

    std::vector <uint8_t> MergeVectorsIntoRepresentationLookupSequence (
        const VolumetricRepresentation &_representation,
        const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_firstVector,
        const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_secondVector) const;

    Collection collection;
    StandardLayout::Mapping typeMapping;

    std::unordered_map <std::string, RepresentationReference> representationReferences;
    std::optional <Collection::Allocator> collectionAllocator;
    std::unordered_map <std::string, Cursor> activeCursors;
};

ExecutionContext::ExecutionContext (const StandardLayout::Mapping &_typeMapping)
    : collection (_typeMapping),
      typeMapping (_typeMapping),
      representationReferences (),
      collectionAllocator (),
      activeCursors ()
{
}

void ExecutionContext::ExecuteTask (const CreatePointRepresentation &_task)
{
    REQUIRE_WITH_MESSAGE (
        representationReferences.find (_task.name) == representationReferences.end (),
        "There should be no representation reference with name \"", _task.name, "\".");

    PointRepresentation representation = collection.CreatePointRepresentation (_task.keyFields);
    // Check that representation key fields are equal to expected key fields.
    std::size_t keyFieldIndex = 0u;

    for (auto iterator = representation.KeyFieldBegin (); iterator != representation.KeyFieldEnd (); ++iterator)
    {
        // Do some unnecessary stuff to cover more iterator operations.
        if (iterator != representation.KeyFieldBegin ())
        {
            auto iteratorCopy = iterator;
            CHECK ((*(iterator--)).IsSame (*iteratorCopy));
            CHECK (iterator++ != iteratorCopy);
            CHECK (iterator == iteratorCopy);
        }

        const bool overflow = keyFieldIndex >= _task.keyFields.size ();
        CHECK (!overflow);

        if (!overflow)
        {
            CHECK (typeMapping.GetField (_task.keyFields[keyFieldIndex]).IsSame (*iterator));
            ++keyFieldIndex;
        }
    }

    representationReferences.emplace (_task.name, representation);
    IterateOverRepresentations ();
}

void ExecutionContext::ExecuteTask (const CreateLinearRepresentation &_task)
{
    REQUIRE_WITH_MESSAGE (
        representationReferences.find (_task.name) == representationReferences.end (),
        "There should be no representation reference with name \"", _task.name, "\".");

    LinearRepresentation representation = collection.CreateLinearRepresentation (_task.keyField);
    CHECK (typeMapping.GetField (_task.keyField).IsSame (representation.GetKeyField ()));

    representationReferences.emplace (_task.name, representation);
    IterateOverRepresentations ();
}

void ExecutionContext::ExecuteTask (const CreateVolumetricRepresentation &_task)
{
    REQUIRE_WITH_MESSAGE (
        representationReferences.find (_task.name) == representationReferences.end (),
        "There should be no representation reference with name \"", _task.name, "\".");

    std::vector <Collection::DimensionDescriptor> convertedDescriptors;
    convertedDescriptors.reserve (_task.dimensions.size ());

    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _task.dimensions)
    {
        convertedDescriptors.emplace_back (
            Collection::DimensionDescriptor
                {
                    &dimension.globalMin,
                    dimension.minField,
                    &dimension.globalMax,
                    dimension.maxField,
                });
    }

    VolumetricRepresentation representation = collection.CreateVolumetricRepresentation (convertedDescriptors);
    // Check that dimensions key fields are equal to expected key fields.
    std::size_t dimensionIndex = 0u;

    for (auto iterator = representation.DimensionBegin (); iterator != representation.DimensionEnd (); ++iterator)
    {
        const bool overflow = dimensionIndex >= _task.dimensions.size ();
        CHECK (!overflow);

        if (!overflow)
        {
            const auto dimension = *iterator;
            CHECK (typeMapping.GetField (_task.dimensions[dimensionIndex].minField).IsSame (dimension.minField));
            CHECK (typeMapping.GetField (_task.dimensions[dimensionIndex].maxField).IsSame (dimension.maxField));
            ++dimensionIndex;
        }
    }

    representationReferences.emplace (_task.name, representation);
    IterateOverRepresentations ();
}

void ExecutionContext::ExecuteTask (const CopyRepresentationReference &_task)
{
    auto iterator = representationReferences.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != representationReferences.end (),
        "There should be representation reference with name \"", _task.sourceName, "\".");

    // Copying reference into itself is ok and may even be used as part of special test scenario.
    representationReferences.emplace (_task.targetName, iterator->second);
}

void ExecutionContext::ExecuteTask (const RemoveRepresentationReference &_task)
{
    auto iterator = representationReferences.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != representationReferences.end (),
        "There should be representation reference with name \"", _task.name, "\".");

    representationReferences.erase (iterator);
}

void ExecutionContext::ExecuteTask (const CheckIsSourceBusy &_task)
{
    auto iterator = representationReferences.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != representationReferences.end (),
        "There should be representation reference with name \"", _task.name, "\".");

    std::visit (
        [&_task] (auto &_reference)
        {
            CHECK_EQUAL (_reference.CanBeDropped (), !_task.expectedValue);
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const DropRepresentation &_task)
{
    auto iterator = representationReferences.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != representationReferences.end (),
        "There should be representation reference with name \"", _task.name, "\".");

    std::visit (
        [] (auto &_reference)
        {
            _reference.Drop ();
        },
        iterator->second);

    representationReferences.erase (iterator);
    IterateOverRepresentations ();
}

void ExecutionContext::ExecuteTask (const OpenAllocator &)
{
    REQUIRE_WITH_MESSAGE (!collectionAllocator, "There should be no active allocator.");
    collectionAllocator.emplace (collection.AllocateAndInsert ());
}

void ExecutionContext::ExecuteTask (const AllocateAndInit &_task)
{
    REQUIRE_WITH_MESSAGE (collectionAllocator, "There should be active allocator.");
    void *record = collectionAllocator.value ().Allocate ();
    CHECK (record != nullptr);

    if (record)
    {
        memcpy (record, _task.copyFrom, typeMapping.GetObjectSize ());
    }
}

void ExecutionContext::ExecuteTask (const CloseAllocator &)
{
    REQUIRE_WITH_MESSAGE (collectionAllocator, "There should be active allocator.");
    collectionAllocator.reset ();
}

void ExecutionContext::ExecuteTask (const QueryValueToRead &_task)
{
    PointRepresentation representation = std::get <PointRepresentation> (PrepareForLookup (_task));
    activeCursors.emplace (_task.cursorName, representation.ReadPoint (_task.value));
}

void ExecutionContext::ExecuteTask (const QueryValueToEdit &_task)
{
    PointRepresentation representation = std::get <PointRepresentation> (PrepareForLookup (_task));
    activeCursors.emplace (_task.cursorName, representation.EditPoint (_task.value));
}

void ExecutionContext::ExecuteTask (const QueryRangeToRead &_task)
{
    LinearRepresentation representation = std::get <LinearRepresentation> (PrepareForLookup (_task));
    activeCursors.emplace (_task.cursorName, representation.ReadInterval (_task.minValue, _task.maxValue));
}

void ExecutionContext::ExecuteTask (const QueryRangeToEdit &_task)
{
    LinearRepresentation representation = std::get <LinearRepresentation> (PrepareForLookup (_task));
    activeCursors.emplace (_task.cursorName, representation.EditInterval (_task.minValue, _task.maxValue));
}

void ExecutionContext::ExecuteTask (const QueryReversedRangeToRead &_task)
{
    LinearRepresentation representation = std::get <LinearRepresentation> (PrepareForLookup (_task));
    activeCursors.emplace (_task.cursorName, representation.ReadReversedInterval (_task.minValue, _task.maxValue));
}

void ExecutionContext::ExecuteTask (const QueryReversedRangeToEdit &_task)
{
    LinearRepresentation representation = std::get <LinearRepresentation> (PrepareForLookup (_task));
    activeCursors.emplace (_task.cursorName, representation.EditReversedInterval (_task.minValue, _task.maxValue));
}

void ExecutionContext::ExecuteTask (const QueryShapeIntersectionToRead &_task)
{
    VolumetricRepresentation representation = std::get <VolumetricRepresentation> (PrepareForLookup (_task));
    std::vector <uint8_t> sequence = MergeVectorsIntoRepresentationLookupSequence (
        representation, _task.min, _task.max);
    activeCursors.emplace (_task.cursorName, representation.ReadShapeIntersections (&sequence[0u]));
}

void ExecutionContext::ExecuteTask (const QueryShapeIntersectionToEdit &_task)
{
    VolumetricRepresentation representation = std::get <VolumetricRepresentation> (PrepareForLookup (_task));
    std::vector <uint8_t> sequence = MergeVectorsIntoRepresentationLookupSequence (
        representation, _task.min, _task.max);
    activeCursors.emplace (_task.cursorName, representation.EditShapeIntersections (&sequence[0u]));
}

void ExecutionContext::ExecuteTask (const QueryRayIntersectionToRead &_task)
{
    VolumetricRepresentation representation = std::get <VolumetricRepresentation> (PrepareForLookup (_task));
    std::vector <uint8_t> sequence = MergeVectorsIntoRepresentationLookupSequence (
        representation, _task.origin, _task.direction);
    activeCursors.emplace (_task.cursorName, representation.ReadRayIntersections (&sequence[0u], _task.maxDistance));
}

void ExecutionContext::ExecuteTask (const QueryRayIntersectionToEdit &_task)
{
    VolumetricRepresentation representation = std::get <VolumetricRepresentation> (PrepareForLookup (_task));
    std::vector <uint8_t> sequence = MergeVectorsIntoRepresentationLookupSequence (
        representation, _task.origin, _task.direction);
    activeCursors.emplace (_task.cursorName, representation.EditRayIntersections (&sequence[0u], _task.maxDistance));
}

void ExecutionContext::ExecuteTask (const CursorCheck &_task)
{
    auto iterator = activeCursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.name, "\".");

    std::visit (
        [this, &_task] (auto &_cursor)
        {
            const void *record = *_cursor;
            if (_task.expectedObject)
            {
                if (record)
                {
                    bool equal = memcmp (record, _task.expectedObject,
                                         typeMapping.GetObjectSize ()) == 0u;

                    CHECK_WITH_MESSAGE (
                        equal,
                        "Expected and pointed records should be equal!\nRecord: ", RecordToString (record),
                        "\nExpected record: ", RecordToString (_task.expectedObject));
                }
                else
                {
                    CHECK_WITH_MESSAGE (false, "Cursor should not be empty!");
                }
            }
            else if (record)
            {
                CHECK_WITH_MESSAGE (false, "Cursor should be empty!");
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorCheckAllOrdered &_task)
{
    auto iterator = activeCursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.name, "\".");

    std::visit (
        [this, &_task] (auto &_cursor)
        {
            std::size_t position = 0u;
            const void *record;

            while ((record = *_cursor) || position < _task.expectedObjects.size ())
            {
                const void *expected;
                if (position < _task.expectedObjects.size ())
                {
                    expected = _task.expectedObjects[position];
                }
                else
                {
                    expected = nullptr;
                }

                if (record && expected)
                {
                    bool equal = memcmp (record, expected, typeMapping.GetObjectSize ()) == 0;
                    CHECK_WITH_MESSAGE (
                        equal, "Checking that received record ", record, " and expected record ", expected,
                        " at position ", position, " are equal.\nReceived: ", RecordToString (record),
                        "\nExpected: ", RecordToString (expected));
                }
                else if (record)
                {
                    CHECK_WITH_MESSAGE (
                        false, "Expecting nothing at position ", position, ", receiving ", RecordToString (record));
                }
                else
                {
                    CHECK_WITH_MESSAGE (
                        false, "Expecting ", expected, " at position ", position, ", but receiving nothing");
                }

                if (record)
                {
                    ++_cursor;
                }

                ++position;
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorCheckAllUnordered &_task)
{
    auto iterator = activeCursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.name, "\".");

    std::visit (
        [this, &_task] (auto &_cursor)
        {
            std::vector <const void *> records;
            while (const void *record = *_cursor)
            {
                records.emplace_back (record);
                ++_cursor;
            }

            CHECK_EQUAL (records.size (), _task.expectedObjects.size ());
            auto Search = [this] (const std::vector <const void *> &_records, const void *_recordToSearch)
            {
                return std::find_if (_records.begin (), _records.end (),
                                     [this, _recordToSearch] (const void *_otherRecord)
                                     {
                                         return memcmp (_recordToSearch, _otherRecord,
                                                        typeMapping.GetObjectSize ()) == 0u;
                                     });
            };

            for (const void *recordFromCursor : records)
            {
                auto iterator = Search (_task.expectedObjects, recordFromCursor);
                if (iterator == _task.expectedObjects.end ())
                {
                    CHECK_WITH_MESSAGE (
                        false, "Searching for record from cursor in expected records list. Record: ",
                        RecordToString (recordFromCursor));
                }
            }

            for (const void *expectedRecord : records)
            {
                auto iterator = Search (records, expectedRecord);
                if (iterator == records.end ())
                {
                    CHECK_WITH_MESSAGE (
                        false, "Searching for expected record in received records list. Record: ",
                        RecordToString (expectedRecord));
                }
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorEdit &_task)
{
    auto iterator = activeCursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.name, "\".");

    std::visit (
        [this, &_task] (auto &_cursor)
        {
            if constexpr (isEditCursor <std::decay_t <decltype (_cursor)>>)
            {
                void *record = *_cursor;
                CHECK_WITH_MESSAGE (record, "Cursor should not be empty.");
                REQUIRE_WITH_MESSAGE (_task.copyFromObject, "New value source must not be null pointer!");

                if (record)
                {
                    memcpy (record, _task.copyFromObject, typeMapping.GetObjectSize ());
                }
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Cursor ", _task.name, " should be editable.");
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorIncrement &_task)
{
    auto iterator = activeCursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.name, "\".");

    std::visit (
        [] (auto &_cursor)
        {
            ++_cursor;
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorDeleteObject &_task)
{
    auto iterator = activeCursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.name, "\".");

    std::visit (
        [&_task] (auto &_cursor)
        {
            if constexpr (isEditCursor <std::decay_t <decltype (_cursor)>>)
            {
                ~_cursor;
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Cursor ", _task.name, " should be editable.");
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorCopy &_task)
{
    auto iterator = activeCursors.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.sourceName, "\".");

    REQUIRE_WITH_MESSAGE (
        activeCursors.find (_task.targetName) == activeCursors.end (),
        "There should be no active cursor with name \"", _task.targetName, "\".");

    std::visit (
        [this, &_task] (auto &_cursor)
        {
            if constexpr (!isEditCursor <std::decay_t <decltype (_cursor)>>)
            {
                activeCursors.emplace (_task.targetName, _cursor);
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false,
                                      "Cursor ", _task.sourceName, " should not be editable.");
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorMove &_task)
{
    auto iterator = activeCursors.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.sourceName, "\".");

    REQUIRE_WITH_MESSAGE (
        activeCursors.find (_task.targetName) == activeCursors.end (),
        "There should be no active cursor with name \"", _task.targetName, "\".");

    std::visit (
        [this, &_task] (auto &_cursor)
        {
            activeCursors.emplace (_task.targetName, std::move (_cursor));
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorClose &_task)
{
    auto iterator = activeCursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.name, "\".");

    activeCursors.erase (iterator);
}

const RepresentationReference &ExecutionContext::PrepareForLookup (const QueryBase &_task) const
{
    REQUIRE_WITH_MESSAGE (
        activeCursors.find (_task.cursorName) == activeCursors.end (),
        "There should be no cursor with name \"", _task.cursorName, "\"");

    auto iterator = representationReferences.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != representationReferences.end (),
        "There should be representation reference with name \"", _task.sourceName, "\".");

    return iterator->second;
}

void ExecutionContext::IterateOverRepresentations () const
{
    std::vector <RepresentationReference> known;
    for (const auto &[name, representation] : representationReferences)
    {
        known.emplace_back (representation);
    }

    std::vector <RepresentationReference> found;
    // During iterations below we will execute some unnecessary operations
    // with iterator to cover more iteration-related operations.

    for (auto iterator = collection.LinearRepresentationBegin ();
         iterator != collection.LinearRepresentationEnd (); ++iterator)
    {
        if (iterator != collection.LinearRepresentationBegin ())
        {
            auto iteratorCopy = iterator;
            CHECK (*(iterator--) == *iteratorCopy);
            CHECK (RepresentationReference (*(iterator++)) == found.back ());
            CHECK (iterator == iteratorCopy);

            --iteratorCopy;
            CHECK (iterator != iteratorCopy);
            iteratorCopy = iterator;
            CHECK (iterator == iteratorCopy);
        }

        found.emplace_back (*iterator);
    }

    for (auto iterator = collection.PointRepresentationBegin ();
         iterator != collection.PointRepresentationEnd (); ++iterator)
    {
        if (iterator != collection.PointRepresentationBegin ())
        {
            auto iteratorCopy = iterator;
            CHECK (*(iterator--) == *iteratorCopy);
            CHECK (RepresentationReference (*(iterator++)) == found.back ());
            CHECK (iterator == iteratorCopy);

            --iteratorCopy;
            CHECK (iterator != iteratorCopy);
            iteratorCopy = iterator;
            CHECK (iterator == iteratorCopy);
        }

        found.emplace_back (*iterator);
    }

    for (auto iterator = collection.VolumetricRepresentationBegin ();
         iterator != collection.VolumetricRepresentationEnd (); ++iterator)
    {
        if (iterator != collection.VolumetricRepresentationBegin ())
        {
            auto iteratorCopy = iterator;
            CHECK (*(iterator--) == *iteratorCopy);
            CHECK (RepresentationReference (*(iterator++)) == found.back ());
            CHECK (iterator == iteratorCopy);

            --iteratorCopy;
            CHECK (iterator != iteratorCopy);
            iteratorCopy = iterator;
            CHECK (iterator == iteratorCopy);
        }

        found.emplace_back (*iterator);
    }

    for (const auto &representation : found)
    {
        CHECK_WITH_MESSAGE (std::find (known.begin (), known.end (), representation) != known.end (),
                            "Searching received representation with address ",
                            *reinterpret_cast <const void *const *> (&representation), " in known list.");
    }

    for (const auto &representation : known)
    {
        CHECK_WITH_MESSAGE (std::find (found.begin (), found.end (), representation) != found.end (),
                            "Searching known representation with address ",
                            *reinterpret_cast <const void *const *> (&representation), " in received list.");
    }
}

std::string ExecutionContext::RecordToString (const void *_record) const
{
    const auto *current = static_cast <const uint8_t *> (_record);
    const auto *end = current + typeMapping.GetObjectSize ();
    std::string result;

    while (current != end)
    {
        result += std::to_string (static_cast <std::size_t> (*current)) + " ";
        ++current;
    }

    return result;
}

std::vector <uint8_t>
ExecutionContext::MergeVectorsIntoRepresentationLookupSequence (
    const VolumetricRepresentation &_representation,
    const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_firstVector,
    const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_secondVector) const
{
    std::size_t sequenceSize = 0u;
    std::size_t dimensionCount = 0u;

    for (auto iterator = _representation.DimensionBegin (); iterator != _representation.DimensionEnd (); ++iterator)
    {
        auto dimension = *iterator;
        REQUIRE (dimension.minField.GetSize () == dimension.maxField.GetSize ());
        REQUIRE (dimension.minField.GetSize () <= sizeof (Query::Test::Sources::Volumetric::SupportedValue));

        sequenceSize += dimension.minField.GetSize () + dimension.maxField.GetSize ();
        ++dimensionCount;
    }

    REQUIRE (_firstVector.size () == dimensionCount);
    REQUIRE (_secondVector.size () == dimensionCount);

    std::vector <uint8_t> sequence (sequenceSize);
    std::size_t dimensionIndex = 0u;
    uint8_t *output = &sequence[0u];

    for (auto iterator = _representation.DimensionBegin (); iterator != _representation.DimensionEnd (); ++iterator)
    {
        auto dimension = *iterator;
        for (std::size_t byteIndex = 0u; byteIndex < dimension.minField.GetSize (); ++byteIndex)
        {
            *output = reinterpret_cast <const uint8_t *> (&_firstVector[dimensionIndex])[byteIndex];
            ++output;
        }

        for (std::size_t byteIndex = 0u; byteIndex < dimension.minField.GetSize (); ++byteIndex)
        {
            *output = reinterpret_cast <const uint8_t *> (&_secondVector[dimensionIndex])[byteIndex];
            ++output;
        }

        ++dimensionIndex;
    }

    return sequence;
}

std::ostream &operator << (std::ostream &_output, const CreateLinearRepresentation &_task)
{
    return _output << "Create linear representation \"" << _task.name << "\" on field " << _task.keyField << ".";
}

std::ostream &operator << (std::ostream &_output, const CreatePointRepresentation &_task)
{
    _output << "Create point representation \"" << _task.name << "\" on fields:";
    for (StandardLayout::FieldId fieldId : _task.keyFields)
    {
        _output << " " << fieldId;
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CreateVolumetricRepresentation &_task)
{
    _output << "Create volumetric representation \"" << _task.name << "\" on dimensions:";
    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _task.dimensions)
    {
        _output << " { globalMin: " << dimension.globalMin << ", minField: " <<
                dimension.minField << ", globalMax: " << dimension.globalMax <<
                ", maxField: " << dimension.maxField << " }";
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CopyRepresentationReference &_task)
{
    return _output << "Copy representation reference \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const RemoveRepresentationReference &_task)
{
    return _output << "Remove representation reference \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const DropRepresentation &_task)
{
    return _output << "Drop representation \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const OpenAllocator &)
{
    return _output << "Open allocator.";
}

std::ostream &operator << (std::ostream &_output, const AllocateAndInit &_task)
{
    return _output << "Allocate record and init from " << _task.copyFrom << ".";
}

std::ostream &operator << (std::ostream &_output, const CloseAllocator &)
{
    return _output << "Close allocator.";
}

static Task CreateRepresentation (const Query::Test::Sources::Value &_source)
{
    return CreatePointRepresentation {_source.name, _source.queriedFields};
}

static Task CreateRepresentation (const Query::Test::Sources::Range &_source)
{
    return CreateLinearRepresentation {_source.name, _source.queriedField};
}

static Task CreateRepresentation (const Query::Test::Sources::Volumetric &_source)
{
    return CreateVolumetricRepresentation {_source.name, _source.dimensions};
}

static void ExecuteQueryApiScenario (const Query::Test::Scenario &_scenario, bool _allocateFirst)
{
    std::vector <Task> tasks;
    // TODO: Augment scenario system to support multiple collections? Do this to Pegasus tests too?

    REQUIRE_WITH_MESSAGE (
        _scenario.storages.size () == 1u,
        "Only one-storage tests are supported right now, because record collections are independent.");

    auto InsertRecords = [&_scenario, &tasks] ()
    {
        tasks.emplace_back (OpenAllocator {});
        for (const void *record : _scenario.storages[0u].objectsToInsert)
        {
            tasks.emplace_back (AllocateAndInit {record});
        }

        tasks.emplace_back (CloseAllocator {});
    };

    auto CreateRepresentations = [&_scenario, &tasks] ()
    {
        for (const Query::Test::Source &source : _scenario.storages[0u].sources)
        {
            std::visit (
                [&tasks] (const auto &_unwrappedSource)
                {
                    tasks.emplace_back (CreateRepresentation (_unwrappedSource));
                },
                source);
        }
    };

    if (_allocateFirst)
    {
        InsertRecords ();
        CreateRepresentations ();
    }
    else
    {
        CreateRepresentations ();
        InsertRecords ();
    }

    for (const Query::Test::Task &task : _scenario.tasks)
    {
        std::visit (
            [&tasks] (const auto &_unwrappedTask)
            {
                tasks.emplace_back (_unwrappedTask);
            },
            task);
    }

    Scenario (_scenario.storages[0u].dataType, tasks);
}

void TestQueryApiDrivers::CreateRepresentationsThanAllocateRecords (const Query::Test::Scenario &_scenario)
{
    ExecuteQueryApiScenario (_scenario, false);
}

void TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (const Query::Test::Scenario &_scenario)
{
    ExecuteQueryApiScenario (_scenario, true);
}

Scenario::Scenario (StandardLayout::Mapping _mapping, std::vector <Task> _tasks)
    : mapping (std::move (_mapping)),
      tasks (std::move (_tasks))
{
    ExecutionContext context (mapping);
    LOG ((std::stringstream () << *this).str ());

    for (const Task &wrappedTask : tasks)
    {
        std::visit (
            [&context] (const auto &_unwrappedTask)
            {
                LOG ((std::stringstream () << _unwrappedTask).str ());
                context.ExecuteTask (_unwrappedTask);
            },
            wrappedTask);
    }
}

std::ostream &operator << (std::ostream &_output, const Scenario &_scenario)
{
    _output << "Scenario: " << std::endl;
    for (const Task &wrappedTask : _scenario.tasks)
    {
        _output << " - ";
        std::visit (
            [&_output] (const auto &_unwrappedTask)
            {
                _output << _unwrappedTask;
            },
            wrappedTask);

        _output << std::endl;
    }

    return _output;
}

std::vector <Task> operator + (std::vector <Task> first, const std::vector <Task> &second) noexcept
{
    first.insert (first.end (), second.begin (), second.end ());
    return first;
}
} // namespace Emergence::RecordCollection::Test