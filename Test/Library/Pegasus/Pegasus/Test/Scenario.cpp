#include <optional>
#include <sstream>
#include <unordered_map>

#include <Pegasus/Storage.hpp>
#include <Pegasus/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Pegasus::Test
{
using IndexReference = std::variant <
    Handling::Handle <HashIndex>,
    Handling::Handle <OrderedIndex>,
    Handling::Handle <VolumetricIndex>>;

using Cursor = std::variant <
    HashIndex::ReadCursor,
    HashIndex::EditCursor,
    OrderedIndex::ReadCursor,
    OrderedIndex::EditCursor,
    OrderedIndex::ReversedReadCursor,
    OrderedIndex::ReversedEditCursor,
    VolumetricIndex::ShapeIntersectionReadCursor,
    VolumetricIndex::ShapeIntersectionEditCursor,
    VolumetricIndex::RayIntersectionReadCursor,
    VolumetricIndex::RayIntersectionEditCursor>;

template <typename CursorType>
constexpr bool isEditCursor =
    std::is_same_v <CursorType, HashIndex::EditCursor> ||
    std::is_same_v <CursorType, OrderedIndex::EditCursor> ||
    std::is_same_v <CursorType, OrderedIndex::ReversedEditCursor> ||
    std::is_same_v <CursorType, VolumetricIndex::ShapeIntersectionEditCursor> ||
    std::is_same_v <CursorType, VolumetricIndex::RayIntersectionEditCursor>;

struct ExecutionContext
{
    explicit ExecutionContext (StandardLayout::Mapping _recordMapping);

    void ExecuteTask (const CreateHashIndex &_task);

    void ExecuteTask (const CreateOrderedIndex &_task);

    void ExecuteTask (const CreateVolumetricIndex &_task);

    void ExecuteTask (const CopyIndexReference &_task);

    void ExecuteTask (const RemoveIndexReference &_task);

    void ExecuteTask (const CheckIndexCanBeDropped &_task);

    void ExecuteTask (const DropIndex &_task);

    void ExecuteTask (const OpenAllocator &);

    void ExecuteTask (const AllocateAndInit &_task);

    void ExecuteTask (const CloseAllocator &);

    void ExecuteTask (const HashIndexLookupToRead &_task);

    void ExecuteTask (const HashIndexLookupToEdit &_task);

    void ExecuteTask (const OrderedIndexLookupToRead &_task);

    void ExecuteTask (const OrderedIndexLookupToEdit &_task);

    void ExecuteTask (const OrderedIndexLookupToReadReversed &_task);

    void ExecuteTask (const OrderedIndexLookupToEditReversed &_task);

    void ExecuteTask (const VolumetricIndexShapeIntersectionLookupToRead &_task);

    void ExecuteTask (const VolumetricIndexShapeIntersectionLookupToEdit &_task);

    void ExecuteTask (const VolumetricIndexRayIntersectionLookupToRead &_task);

    void ExecuteTask (const VolumetricIndexRayIntersectionLookupToEdit &_task);

    void ExecuteTask (const CursorCheck &_task);

    void ExecuteTask (const CursorCheckAllOrdered &_task);

    void ExecuteTask (const CursorCheckAllUnordered &_task);

    void ExecuteTask (const CursorEdit &_task);

    void ExecuteTask (const CursorIncrement &_task);

    void ExecuteTask (const CursorDeleteRecord &_task);

    void ExecuteTask (const CopyCursor &_task);

    void ExecuteTask (const MoveCursor &_task);

    void ExecuteTask (const CloseCursor &_task);

    const IndexReference &PrepareForLookup (const IndexLookupBase &_task) const;

    void IterateOverIndices () const;

    void OnIndexDropped (HashIndex *_index);

    void OnIndexDropped (OrderedIndex *_index);

    void OnIndexDropped (VolumetricIndex *_index);

    std::string RecordToString (const void *_record) const;

    VolumetricIndex::AxisAlignedShapeContainer ExtractShape (
        const VolumetricIndex *_index, const VolumetricIndexShapeIntersectionLookupBase &_task) const;

    VolumetricIndex::RayContainer ExtractRay (
        const VolumetricIndex *_index, const VolumetricIndexRayIntersectionLookupBase &_task) const;

    Storage storage;
    std::vector <HashIndex *> knownHashIndices;
    std::vector <OrderedIndex *> knownOrderedIndices;
    std::vector <VolumetricIndex *> knownVolumetricIndices;

    std::unordered_map <std::string, IndexReference> indexReferences;
    std::optional <Storage::Allocator> storageAllocator;
    std::unordered_map <std::string, Cursor> activeCursors;
};

ExecutionContext::ExecutionContext (StandardLayout::Mapping _recordMapping)
    : storage (std::move (_recordMapping)),
      knownHashIndices (),
      knownOrderedIndices (),
      knownVolumetricIndices (),
      indexReferences ()
{
}

void ExecutionContext::ExecuteTask (const CreateHashIndex &_task)
{
    REQUIRE_WITH_MESSAGE (
        indexReferences.find (_task.name) == indexReferences.end (),
        "There should be no index reference with name \"", _task.name, "\".");

    Handling::Handle <HashIndex> index = storage.CreateHashIndex (_task.indexedFields);
    REQUIRE_WITH_MESSAGE (index, "Returned index should not be null.");

    knownHashIndices.emplace_back (index.Get ());
    indexReferences.emplace (_task.name, index);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const CreateOrderedIndex &_task)
{
    REQUIRE_WITH_MESSAGE (
        indexReferences.find (_task.name) == indexReferences.end (),
        "There should be no index reference with name \"", _task.name, "\".");

    Handling::Handle <OrderedIndex> index = storage.CreateOrderedIndex (_task.indexedField);
    REQUIRE_WITH_MESSAGE (index, "Returned index should not be null.");

    knownOrderedIndices.emplace_back (index.Get ());
    indexReferences.emplace (_task.name, index);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const CreateVolumetricIndex &_task)
{
    REQUIRE_WITH_MESSAGE (
        indexReferences.find (_task.name) == indexReferences.end (),
        "There should be no index reference with name \"", _task.name, "\".");

    std::vector <VolumetricIndex::DimensionDescriptor> convertedDescriptors;
    convertedDescriptors.reserve (_task.dimensions.size ());

    for (const DimensionDescriptor &dimension : _task.dimensions)
    {
        convertedDescriptors.emplace_back (
            VolumetricIndex::DimensionDescriptor
                {
                    *static_cast <const VolumetricIndex::SupportedAxisValue *> (dimension.globalMinBorderValue),
                    dimension.minBorderField,
                    *static_cast <const VolumetricIndex::SupportedAxisValue *> (dimension.globalMaxBorderValue),
                    dimension.maxBorderField,
                });
    }

    Handling::Handle <VolumetricIndex> index = storage.CreateVolumetricIndex (convertedDescriptors);
    REQUIRE_WITH_MESSAGE (index, "Returned index should not be null.");

    knownVolumetricIndices.emplace_back (index.Get ());
    indexReferences.emplace (_task.name, index);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const CopyIndexReference &_task)
{
    auto iterator = indexReferences.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.sourceName, "\".");

    // Copying reference into itself is ok and may even be used as part of special test scenario.
    indexReferences.emplace (_task.targetName, iterator->second);
}

void ExecutionContext::ExecuteTask (const RemoveIndexReference &_task)
{
    auto iterator = indexReferences.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.name, "\".");

    indexReferences.erase (iterator);
}

void ExecutionContext::ExecuteTask (const CheckIndexCanBeDropped &_task)
{
    auto iterator = indexReferences.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.name, "\".");

    std::visit (
        [&_task] (auto &_handle)
        {
            auto *handleValue = _handle.Get ();
            // Temporary make handle free. Otherwise CanBeDropped check will always return false.
            _handle = nullptr;
            CHECK_EQUAL (handleValue->CanBeDropped (), _task.expectedResult);
            _handle = handleValue;
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const DropIndex &_task)
{
    auto iterator = indexReferences.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.name, "\".");

    std::visit (
        [this] (auto &_handle)
        {
            auto *handleValue = _handle.Get ();
            // Firstly make handle free. Otherwise CanBeDropped check will always return false.
            _handle = nullptr;

            REQUIRE (handleValue->CanBeDropped ());
            handleValue->Drop ();
            OnIndexDropped (handleValue);
        },
        iterator->second);

    indexReferences.erase (iterator);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const OpenAllocator &)
{
    REQUIRE_WITH_MESSAGE (!storageAllocator, "There should be no active allocator.");
    storageAllocator.emplace (storage.AllocateAndInsert ());
}

void ExecutionContext::ExecuteTask (const AllocateAndInit &_task)
{
    REQUIRE_WITH_MESSAGE (storageAllocator, "There should be active allocator.");
    void *record = storageAllocator.value ().Next ();
    CHECK (record != nullptr);

    if (record)
    {
        memcpy (record, _task.copyFrom, storage.GetRecordMapping ().GetObjectSize ());
    }
}

void ExecutionContext::ExecuteTask (const CloseAllocator &)
{
    REQUIRE_WITH_MESSAGE (storageAllocator, "There should be active allocator.");
    storageAllocator.reset ();
}

void ExecutionContext::ExecuteTask (const HashIndexLookupToRead &_task)
{
    HashIndex *index = std::get <Handling::Handle <HashIndex>> (PrepareForLookup (_task)).Get ();
    activeCursors.emplace (_task.cursorName, index->LookupToRead ({_task.request}));
}

void ExecutionContext::ExecuteTask (const HashIndexLookupToEdit &_task)
{
    HashIndex *index = std::get <Handling::Handle <HashIndex>> (PrepareForLookup (_task)).Get ();
    activeCursors.emplace (_task.cursorName, index->LookupToEdit ({_task.request}));
}

void ExecutionContext::ExecuteTask (const OrderedIndexLookupToRead &_task)
{
    OrderedIndex *index = std::get <Handling::Handle <OrderedIndex>> (PrepareForLookup (_task)).Get ();
    activeCursors.emplace (_task.cursorName, index->LookupToRead ({_task.minValue}, {_task.maxValue}));
}

void ExecutionContext::ExecuteTask (const OrderedIndexLookupToEdit &_task)
{
    OrderedIndex *index = std::get <Handling::Handle <OrderedIndex>> (PrepareForLookup (_task)).Get ();
    activeCursors.emplace (_task.cursorName, index->LookupToEdit ({_task.minValue}, {_task.maxValue}));
}

void ExecutionContext::ExecuteTask (const OrderedIndexLookupToReadReversed &_task)
{
    OrderedIndex *index = std::get <Handling::Handle <OrderedIndex>> (PrepareForLookup (_task)).Get ();
    activeCursors.emplace (
        _task.cursorName, index->LookupToReadReversed ({_task.minValue}, {_task.maxValue}));
}

void ExecutionContext::ExecuteTask (const OrderedIndexLookupToEditReversed &_task)
{
    OrderedIndex *index = std::get <Handling::Handle <OrderedIndex>> (PrepareForLookup (_task)).Get ();
    activeCursors.emplace (
        _task.cursorName, index->LookupToEditReversed ({_task.minValue}, {_task.maxValue}));
}

void ExecutionContext::ExecuteTask (const VolumetricIndexShapeIntersectionLookupToRead &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    REQUIRE_EQUAL (_task.min.size (), index->GetDimensions ().GetCount ());
    activeCursors.emplace (_task.cursorName, index->LookupShapeIntersectionToRead (
        ExtractShape (index, _task)));
}

void ExecutionContext::ExecuteTask (const VolumetricIndexShapeIntersectionLookupToEdit &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    REQUIRE_EQUAL (_task.min.size (), index->GetDimensions ().GetCount ());
    activeCursors.emplace (_task.cursorName, index->LookupShapeIntersectionToEdit (
        ExtractShape (index, _task)));
}

void ExecutionContext::ExecuteTask (const VolumetricIndexRayIntersectionLookupToRead &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    REQUIRE_EQUAL (_task.origin.size (), index->GetDimensions ().GetCount ());
    activeCursors.emplace (_task.cursorName, index->LookupRayIntersectionToRead (
        ExtractRay (index, _task), _task.maxDistance));
}

void ExecutionContext::ExecuteTask (const VolumetricIndexRayIntersectionLookupToEdit &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    REQUIRE_EQUAL (_task.origin.size (), index->GetDimensions ().GetCount ());
    activeCursors.emplace (_task.cursorName, index->LookupRayIntersectionToEdit (
        ExtractRay (index, _task), _task.maxDistance));
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
            if (_task.expectedRecord)
            {
                if (record)
                {
                    bool equal = memcmp (record, _task.expectedRecord,
                                         storage.GetRecordMapping ().GetObjectSize ()) == 0u;

                    CHECK_WITH_MESSAGE (
                        equal,
                        "Expected and pointed records should be equal!\nRecord: ", RecordToString (record),
                        "\nExpected record: ", RecordToString (_task.expectedRecord));
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

            while ((record = *_cursor) || position < _task.expectedRecords.size ())
            {
                const void *expected;
                if (position < _task.expectedRecords.size ())
                {
                    expected = _task.expectedRecords[position];
                }
                else
                {
                    expected = nullptr;
                }

                if (record && expected)
                {
                    bool equal = memcmp (record, expected, storage.GetRecordMapping ().GetObjectSize ()) == 0;
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

            CHECK_EQUAL (records.size (), _task.expectedRecords.size ());
            auto Search = [this] (const std::vector <const void *> &_records, const void *_recordToSearch)
            {
                return std::find_if (_records.begin (), _records.end (),
                                     [this, _recordToSearch] (const void *_otherRecord)
                                     {
                                         return memcmp (_recordToSearch, _otherRecord,
                                                        storage.GetRecordMapping ().GetObjectSize ()) == 0u;
                                     });
            };

            for (const void *recordFromCursor : records)
            {
                auto iterator = Search (_task.expectedRecords, recordFromCursor);
                if (iterator == _task.expectedRecords.end ())
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
                REQUIRE_WITH_MESSAGE (_task.copyFrom, "New value source must not be null pointer!");

                if (record)
                {
                    memcpy (record, _task.copyFrom, storage.GetRecordMapping ().GetObjectSize ());
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

void ExecutionContext::ExecuteTask (const CursorDeleteRecord &_task)
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

void ExecutionContext::ExecuteTask (const CopyCursor &_task)
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

void ExecutionContext::ExecuteTask (const MoveCursor &_task)
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

void ExecutionContext::ExecuteTask (const CloseCursor &_task)
{
    auto iterator = activeCursors.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != activeCursors.end (),
        "There should be active cursor with name \"", _task.name, "\".");

    activeCursors.erase (iterator);
}

const IndexReference &ExecutionContext::PrepareForLookup (const IndexLookupBase &_task) const
{
    REQUIRE_WITH_MESSAGE (
        activeCursors.find (_task.cursorName) == activeCursors.end (),
        "There should be no cursor with name \"", _task.cursorName, "\"");

    auto iterator = indexReferences.find (_task.indexName);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.indexName, "\".");

    return iterator->second;
}

void ExecutionContext::IterateOverIndices () const
{
    auto Iterate = [] (auto _current, auto _end, const auto &_known)
    {
        std::decay_t <decltype (_known)> found;
        while (_current != _end)
        {
            found.emplace_back ((*_current).Get ());
            ++_current;
        }

        CHECK_EQUAL (found.size (), _known.size ());
        for (const auto &index : found)
        {
            CHECK_WITH_MESSAGE (std::find (_known.begin (), _known.end (), index) != _known.end (),
                                "Searching received index with address ", index, " in known list.");
        }

        for (const auto &index : _known)
        {
            CHECK_WITH_MESSAGE (std::find (found.begin (), found.end (), index) != found.end (),
                                "Searching known index with address ", index, " in received list.");
        }
    };

    LOG ("Checking hash indices.");
    Iterate (storage.BeginHashIndices (), storage.EndHashIndices (), knownHashIndices);

    LOG ("Checking ordered indices.");
    Iterate (storage.BeginOrderedIndices (), storage.EndOrderedIndices (), knownOrderedIndices);

    LOG ("Checking volumetric indices.");
    Iterate (storage.BeginVolumetricIndices (), storage.EndVolumetricIndices (), knownVolumetricIndices);
}

void ExecutionContext::OnIndexDropped (HashIndex *_index)
{
    auto iterator = std::find (knownHashIndices.begin (), knownHashIndices.end (), _index);
    CHECK (iterator != knownHashIndices.end ());

    if (iterator != knownHashIndices.end ())
    {
        knownHashIndices.erase (iterator);
    }
}

void ExecutionContext::OnIndexDropped (OrderedIndex *_index)
{
    auto iterator = std::find (knownOrderedIndices.begin (), knownOrderedIndices.end (), _index);
    CHECK (iterator != knownOrderedIndices.end ());

    if (iterator != knownOrderedIndices.end ())
    {
        knownOrderedIndices.erase (iterator);
    }
}

void ExecutionContext::OnIndexDropped (VolumetricIndex *_index)
{
    auto iterator = std::find (knownVolumetricIndices.begin (), knownVolumetricIndices.end (), _index);
    CHECK (iterator != knownVolumetricIndices.end ());

    if (iterator != knownVolumetricIndices.end ())
    {
        knownVolumetricIndices.erase (iterator);
    }
}

std::string ExecutionContext::RecordToString (const void *_record) const
{
    const auto *current = static_cast <const uint8_t *> (_record);
    const auto *end = current + storage.GetRecordMapping ().GetObjectSize ();
    std::string result;

    while (current != end)
    {
        result += std::to_string (static_cast <std::size_t> (*current)) + " ";
        ++current;
    }

    return result;
}

template <typename Callback>
auto WithPrimitiveFieldType (const StandardLayout::Field &_field, const Callback &_callback)
{
    REQUIRE (_field.IsHandleValid ());
    switch (_field.GetArchetype ())
    {
        case StandardLayout::FieldArchetype::INT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (int8_t):
                    return _callback (int8_t {});

                case sizeof (int16_t):
                    return _callback (int16_t {});

                case sizeof (int32_t):
                    return _callback (int32_t {});

                case sizeof (int64_t):
                    return _callback (int64_t {});
            }

            break;
        }
        case StandardLayout::FieldArchetype::UINT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (uint8_t):
                    return _callback (uint8_t {});

                case sizeof (uint16_t):
                    return _callback (uint16_t {});

                case sizeof (uint32_t):
                    return _callback (uint32_t {});

                case sizeof (uint64_t):
                    return _callback (uint64_t {});
            }

            break;
        }

        case StandardLayout::FieldArchetype::FLOAT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (float):
                    return _callback (float {});

                case sizeof (double):
                    return _callback (double {});
            }

            break;
        }

        case StandardLayout::FieldArchetype::BIT:
        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
        case StandardLayout::FieldArchetype::STRING:
        {
            break;
        }
    }

    REQUIRE_WITH_MESSAGE(false, "Field type should be primitive!");
    return _callback (float {});
}

VolumetricIndex::AxisAlignedShapeContainer ExecutionContext::ExtractShape (
    const VolumetricIndex *_index, const VolumetricIndexShapeIntersectionLookupBase &_task) const
{
    REQUIRE_EQUAL (_task.min.size (), _task.max.size ());
    REQUIRE (_task.min.size () > 0u);
    REQUIRE (_task.min.size () <= Constants::VolumetricIndex::MAX_DIMENSIONS);

    REQUIRE (_index);
    REQUIRE (!_index->GetDimensions ().Empty ());

    return WithPrimitiveFieldType (
        _index->GetDimensions ()[0u].minBorderField,
        [&_task] (auto _typeInstance) -> VolumetricIndex::AxisAlignedShapeContainer
        {
            using ValueType = std::decay_t <decltype (_typeInstance)>;
            VolumetricIndex::AxisAlignedShape <ValueType> shape {};

            for (std::size_t dimensionIndex = 0u; dimensionIndex < _task.min.size (); ++dimensionIndex)
            {
                shape.Min (dimensionIndex) = *reinterpret_cast<const ValueType *> (_task.min[dimensionIndex]);
                shape.Max (dimensionIndex) = *reinterpret_cast<const ValueType *> (_task.max[dimensionIndex]);
            }

            return *reinterpret_cast <VolumetricIndex::AxisAlignedShapeContainer *> (&shape);
        });
}

VolumetricIndex::RayContainer ExecutionContext::ExtractRay (
    const VolumetricIndex *_index, const VolumetricIndexRayIntersectionLookupBase &_task) const
{
    REQUIRE_EQUAL (_task.origin.size (), _task.direction.size ());
    REQUIRE (_task.origin.size () > 0u);
    REQUIRE (_task.origin.size () <= Constants::VolumetricIndex::MAX_DIMENSIONS);

    REQUIRE (_index);
    REQUIRE (!_index->GetDimensions ().Empty ());

    return WithPrimitiveFieldType (
        _index->GetDimensions ()[0u].minBorderField,
        [&_task] (auto _typeInstance) -> VolumetricIndex::RayContainer
        {
            using ValueType = std::decay_t <decltype (_typeInstance)>;
            VolumetricIndex::Ray <ValueType> ray {};

            for (std::size_t dimensionIndex = 0u; dimensionIndex < _task.origin.size (); ++dimensionIndex)
            {
                ray.Origin (dimensionIndex) = *reinterpret_cast<const ValueType *> (_task.origin[dimensionIndex]);
                ray.Direction (dimensionIndex) = *reinterpret_cast<const ValueType *> (_task.direction[dimensionIndex]);
            }

            return *reinterpret_cast <VolumetricIndex::RayContainer *> (&ray);
        });
}

std::ostream &operator << (std::ostream &_output, const CreateHashIndex &_task)
{
    _output << "Create hash index \"" << _task.name << "\" on fields:";
    for (StandardLayout::FieldId fieldId : _task.indexedFields)
    {
        _output << " " << fieldId;
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CreateOrderedIndex &_task)
{
    return _output << "Create ordered index \"" << _task.name << "\" on field " << _task.indexedField << ".";
}

std::ostream &operator << (std::ostream &_output, const CreateVolumetricIndex &_task)
{
    _output << "Create volumetric index \"" << _task.name << "\" on dimensions:";
    for (const DimensionDescriptor &dimension : _task.dimensions)
    {
        _output << " { globalMinPointer: " << dimension.globalMinBorderValue << ", minField: " <<
                dimension.minBorderField << ", globalMaxPointer: " << dimension.globalMaxBorderValue <<
                ", maxField: " << dimension.maxBorderField << " }";
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CopyIndexReference &_task)
{
    return _output << "Copy index reference \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const RemoveIndexReference &_task)
{
    return _output << "Remove index reference \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const CheckIndexCanBeDropped &_task)
{
    return _output << "Check is index \"" << _task.name << "\" can be dropped, expected result: \"" <<
                   (_task.expectedResult ? "yes" : "no") << "\".";
}

std::ostream &operator << (std::ostream &_output, const DropIndex &_task)
{
    return _output << "Drop index \"" << _task.name << "\".";
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

std::ostream &operator << (std::ostream &_output, const HashIndexLookupToRead &_task)
{
    return _output << "Execute hash index \"" << _task.indexName << "\" read-only lookup using request " <<
                   _task.request << " and save cursor as \"" << _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const HashIndexLookupToEdit &_task)
{
    return _output << "Execute hash index \"" << _task.indexName << "\" editable lookup using request " <<
                   _task.request << " and save cursor as \"" << _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const OrderedIndexLookupToRead &_task)
{
    return _output << "Execute ordered index \"" << _task.indexName <<
                   "\" read-only lookup using request {min = " << _task.minValue <<
                   ", max = " << _task.maxValue << "} and save cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const OrderedIndexLookupToEdit &_task)
{
    return _output << "Execute ordered index \"" << _task.indexName <<
                   "\" editable lookup using request {min = " << _task.minValue <<
                   ", max = " << _task.maxValue << "} and save cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const OrderedIndexLookupToReadReversed &_task)
{
    return _output << "Execute ordered index \"" << _task.indexName <<
                   "\" read-only lookup with reversed order using request {min = " << _task.minValue <<
                   ", max = " << _task.maxValue << "} and save cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const OrderedIndexLookupToEditReversed &_task)
{
    return _output << "Execute ordered index \"" << _task.indexName <<
                   "\" editable lookup with reversed order using request {min = " << _task.minValue <<
                   ", max = " << _task.maxValue << "} and save cursor as \"" <<
                   _task.cursorName << "\".";
}

std::ostream &PrintShapeIntersectionLookupRequest (
    const VolumetricIndexShapeIntersectionLookupBase &_lookup, bool _editable, std::ostream &_output)
{
    _output << "Execute volumetric index \"" << _lookup.indexName << "\" " << (_editable ? "editable" : "readonly") <<
            " lookup using shape { minPointers: { ";

    for (const void *pointer : _lookup.min)
    {
        _output << pointer << " ";
    }

    _output << "}, maxPointers: { ";
    for (const void *pointer : _lookup.max)
    {
        _output << pointer << " ";
    }

    return _output << "} } and save cursor as \"" << _lookup.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const VolumetricIndexShapeIntersectionLookupToRead &_task)
{
    return PrintShapeIntersectionLookupRequest (_task, false, _output);
}

std::ostream &operator << (std::ostream &_output, const VolumetricIndexShapeIntersectionLookupToEdit &_task)
{
    return PrintShapeIntersectionLookupRequest (_task, true, _output);
}

std::ostream &PrintRayIntersectionLookupRequest (
    const VolumetricIndexRayIntersectionLookupBase &_lookup, bool _editable, std::ostream &_output)
{
    _output << "Execute volumetric index \"" << _lookup.indexName << "\" " << (_editable ? "editable" : "readonly") <<
            " lookup using ray { originPointers: { ";

    for (const void *pointer : _lookup.origin)
    {
        _output << pointer << " ";
    }

    _output << "}, directionPointers: { ";
    for (const void *pointer : _lookup.direction)
    {
        _output << pointer << " ";
    }

    return _output << "}, maxDistance " << _lookup.maxDistance <<
                   " } and save cursor as \"" << _lookup.cursorName << "\".";
}

std::ostream &operator << (std::ostream &_output, const VolumetricIndexRayIntersectionLookupToRead &_task)
{
    return PrintRayIntersectionLookupRequest (_task, false, _output);
}

std::ostream &operator << (std::ostream &_output, const VolumetricIndexRayIntersectionLookupToEdit &_task)
{
    return PrintRayIntersectionLookupRequest (_task, true, _output);
}

std::ostream &operator << (std::ostream &_output, const CursorCheck &_task)
{
    return _output << "Check that cursor \"" << _task.name << "\" points to record, equal to " <<
                   _task.expectedRecord << ".";
}

std::ostream &operator << (std::ostream &_output, const CursorCheckAllOrdered &_task)
{
    _output << "Check that cursor \"" << _task.name << "\" points to ordered sequence of records equal to:";
    for (const void *record : _task.expectedRecords)
    {
        _output << " " << record;
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CursorCheckAllUnordered &_task)
{
    _output << "Check that cursor \"" << _task.name << "\" points to set of records equal to:";
    for (const void *record : _task.expectedRecords)
    {
        _output << " " << record;
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CursorEdit &_task)
{
    return _output << "Replace value of record, to which cursor \"" << _task.name <<
                   "\" points with value of " << _task.copyFrom << ".";
}

std::ostream &operator << (std::ostream &_output, const CursorIncrement &_task)
{
    return _output << "Increment cursor \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const CursorDeleteRecord &_task)
{
    return _output << "Delete record, to which cursor \"" << _task.name << "\" points.";
}

std::ostream &operator << (std::ostream &_output, const CopyCursor &_task)
{
    return _output << "Copy cursor \"" << _task.sourceName << "\" as \"" << _task.targetName << ".";
}

std::ostream &operator << (std::ostream &_output, const MoveCursor &_task)
{
    return _output << "Move cursor \"" << _task.sourceName << "\" to \"" << _task.targetName << ".";
}

std::ostream &operator << (std::ostream &_output, const CloseCursor &_task)
{
    return _output << "Close cursor \"" << _task.name << "\".";
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
} // namespace Emergence::Pegasus::Test