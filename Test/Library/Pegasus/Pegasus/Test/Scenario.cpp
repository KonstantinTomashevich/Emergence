#define _CRT_SECURE_NO_WARNINGS

#include <optional>
#include <unordered_map>

#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <Pegasus/Storage.hpp>
#include <Pegasus/Test/Scenario.hpp>

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

    VolumetricIndex::AxisAlignedShape ExtractShape (const VolumetricIndexShapeIntersectionLookupBase &_task) const;

    VolumetricIndex::Ray ExtractRay (const VolumetricIndexRayIntersectionLookupBase &_task) const;

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
    BOOST_REQUIRE_MESSAGE (
        indexReferences.find (_task.name) == indexReferences.end (),
        boost::format ("There should be no index reference with name \"%1%\"") % _task.name);

    Handling::Handle <HashIndex> index = storage.CreateHashIndex (_task.indexedFields);
    BOOST_REQUIRE_MESSAGE (index, "Returned index should not be null.");

    knownHashIndices.emplace_back (index.Get ());
    indexReferences.emplace (_task.name, index);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const CreateOrderedIndex &_task)
{
    BOOST_REQUIRE_MESSAGE (
        indexReferences.find (_task.name) == indexReferences.end (),
        boost::format ("There should be no index reference with name \"%1%\"") % _task.name);

    Handling::Handle <OrderedIndex> index = storage.CreateOrderedIndex (_task.indexedField);
    BOOST_REQUIRE_MESSAGE (index, "Returned index should not be null.");

    knownOrderedIndices.emplace_back (index.Get ());
    indexReferences.emplace (_task.name, index);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const CreateVolumetricIndex &_task)
{
    BOOST_REQUIRE_MESSAGE (
        indexReferences.find (_task.name) == indexReferences.end (),
        boost::format ("There should be no index reference with name \"%1%\"") % _task.name);

    std::vector <VolumetricIndex::DimensionDescriptor> convertedDescriptors;
    convertedDescriptors.reserve (_task.dimensions.size ());

    for (const DimensionDescriptor &dimension : _task.dimensions)
    {
        convertedDescriptors.emplace_back (
            VolumetricIndex::DimensionDescriptor
                {
                    dimension.minBorderField,
                    *static_cast <const VolumetricIndex::SupportedAxisValue *> (dimension.globalMinBorderValue),
                    dimension.maxBorderField,
                    *static_cast <const VolumetricIndex::SupportedAxisValue *> (dimension.globalMaxBorderValue)
                });
    }

    Handling::Handle <VolumetricIndex> index = storage.CreateVolumetricIndex (convertedDescriptors);
    BOOST_REQUIRE_MESSAGE (index, "Returned index should not be null.");

    knownVolumetricIndices.emplace_back (index.Get ());
    indexReferences.emplace (_task.name, index);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const CopyIndexReference &_task)
{
    auto iterator = indexReferences.find (_task.sourceName);
    BOOST_REQUIRE_MESSAGE (
        iterator != indexReferences.end (),
        boost::format ("There should be index reference with name \"%1%\".") % _task.sourceName);

    // Copying reference into itself is ok and may even be used as part of special test scenario.
    indexReferences.emplace (_task.targetName, iterator->second);
}

void ExecutionContext::ExecuteTask (const RemoveIndexReference &_task)
{
    auto iterator = indexReferences.find (_task.name);
    BOOST_REQUIRE_MESSAGE (
        iterator != indexReferences.end (),
        boost::format ("There should be index reference with name \"%1%\".") % _task.name);

    indexReferences.erase (iterator);
}

void ExecutionContext::ExecuteTask (const CheckIndexCanBeDropped &_task)
{
    auto iterator = indexReferences.find (_task.name);
    BOOST_REQUIRE_MESSAGE (
        iterator != indexReferences.end (),
        boost::format ("There should be index reference with name \"%1%\".") % _task.name);

    std::visit (
        [&_task] (auto &_handle)
        {
            auto *handleValue = _handle.Get ();
            // Temporary make handle free. Otherwise CanBeDropped check will always return false.
            _handle = nullptr;
            BOOST_CHECK_EQUAL (handleValue->CanBeDropped (), _task.expectedResult);
            _handle = handleValue;
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const DropIndex &_task)
{
    auto iterator = indexReferences.find (_task.name);
    BOOST_REQUIRE_MESSAGE (
        iterator != indexReferences.end (),
        boost::format ("There should be index reference with name \"%1%\".") % _task.name);

    std::visit (
        [this] (auto &_handle)
        {
            auto *handleValue = _handle.Get ();
            // Firstly make handle free. Otherwise CanBeDropped check will always return false.
            _handle = nullptr;

            BOOST_REQUIRE (handleValue->CanBeDropped ());
            handleValue->Drop ();
            OnIndexDropped (handleValue);
        },
        iterator->second);

    indexReferences.erase (iterator);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const OpenAllocator &)
{
    BOOST_REQUIRE_MESSAGE (!storageAllocator, "There should be no active allocator.");
    storageAllocator.emplace (storage.AllocateAndInsert ());
}

void ExecutionContext::ExecuteTask (const AllocateAndInit &_task)
{
    BOOST_REQUIRE_MESSAGE (storageAllocator, "There should be active allocator.");
    void *record = storageAllocator.value ().Next ();
    BOOST_CHECK_NE (record, nullptr);

    if (record)
    {
        memcpy (record, _task.copyFrom, storage.GetRecordMapping ().GetObjectSize ());
    }
}

void ExecutionContext::ExecuteTask (const CloseAllocator &)
{
    BOOST_REQUIRE_MESSAGE (storageAllocator, "There should be active allocator.");
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
    BOOST_REQUIRE_EQUAL (_task.min.size (), index->GetDimensions ().GetCount ());
    activeCursors.emplace (_task.cursorName, index->LookupShapeIntersectionToRead (ExtractShape (_task)));
}

void ExecutionContext::ExecuteTask (const VolumetricIndexShapeIntersectionLookupToEdit &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    BOOST_REQUIRE_EQUAL (_task.min.size (), index->GetDimensions ().GetCount ());
    activeCursors.emplace (_task.cursorName, index->LookupShapeIntersectionToEdit (ExtractShape (_task)));
}

void ExecutionContext::ExecuteTask (const VolumetricIndexRayIntersectionLookupToRead &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    BOOST_REQUIRE_EQUAL (_task.origin.size (), index->GetDimensions ().GetCount ());
    activeCursors.emplace (_task.cursorName, index->LookupRayIntersectionToRead (ExtractRay (_task)));
}

void ExecutionContext::ExecuteTask (const VolumetricIndexRayIntersectionLookupToEdit &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    BOOST_REQUIRE_EQUAL (_task.origin.size (), index->GetDimensions ().GetCount ());
    activeCursors.emplace (_task.cursorName, index->LookupRayIntersectionToEdit (ExtractRay (_task)));
}

void ExecutionContext::ExecuteTask (const CursorCheck &_task)
{
    auto iterator = activeCursors.find (_task.name);
    BOOST_REQUIRE_MESSAGE (
        iterator != activeCursors.end (),
        boost::format ("There should be active cursor with name \"%1%\".") % _task.name);

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

                    // Do not print check message unless check failed, because check message generation is quite slow.
                    if (!equal)
                    {
                        BOOST_CHECK_MESSAGE (
                            false, boost::format (
                            "Expected and pointed records should be equal!\nRecord: %1%\nExpected record: %2%") %
                            RecordToString (record) % RecordToString (_task.expectedRecord));
                    }
                }
                else
                {
                    BOOST_CHECK_MESSAGE (false, "Cursor should not be empty!");
                }
            }
            else if (record)
            {
                BOOST_CHECK_MESSAGE (false, "Cursor should be empty!");
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorCheckAllOrdered &_task)
{
    auto iterator = activeCursors.find (_task.name);
    BOOST_REQUIRE_MESSAGE (
        iterator != activeCursors.end (),
        boost::format ("There should be active cursor with name \"%1%\".") % _task.name);

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

                    // Do not print check message unless check failed, because check message generation is quite slow.
                    if (!equal)
                    {
                        BOOST_CHECK_MESSAGE (
                            equal, boost::format (
                            "Checking tha received record %1% and expected record %2% at position %3% are equal.\n"
                            "Received: %4%\nExpected: %5%") %
                            record % expected % position % RecordToString (record) % RecordToString (expected));
                    }
                }
                else if (record)
                {
                    BOOST_CHECK_MESSAGE (
                        false,
                        boost::format ("Expecting nothing at position %1%, receiving %2%.") % position % record);
                }
                else
                {
                    BOOST_CHECK_MESSAGE (
                        false,
                        boost::format ("Expecting %1% at position %2%, but receiving nothing") %
                        expected % position);
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
    BOOST_REQUIRE_MESSAGE (
        iterator != activeCursors.end (),
        boost::format ("There should be active cursor with name \"%1%\".") % _task.name);

    std::visit (
        [this, &_task] (auto &_cursor)
        {
            std::vector <const void *> records;
            while (const void *record = *_cursor)
            {
                records.emplace_back (record);
                ++_cursor;
            }

            BOOST_CHECK_EQUAL (records.size (), _task.expectedRecords.size ());
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
                    BOOST_CHECK_MESSAGE(
                        false,
                        boost::format ("Searching for record from cursor in expected records list. Record: %1%") %
                        RecordToString (recordFromCursor));
                }
            }

            for (const void *expectedRecord : records)
            {
                auto iterator = Search (records, expectedRecord);
                if (iterator == records.end ())
                {
                    BOOST_CHECK_MESSAGE(
                        false,
                        boost::format ("Searching for expected record in received records list. Record: %1%") %
                        RecordToString (expectedRecord));
                }
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorEdit &_task)
{
    auto iterator = activeCursors.find (_task.name);
    BOOST_REQUIRE_MESSAGE (
        iterator != activeCursors.end (),
        boost::format ("There should be active cursor with name \"%1%\".") % _task.name);

    std::visit (
        [this, &_task] (auto &_cursor)
        {
            if constexpr (isEditCursor <std::decay_t <decltype (_cursor)>>)
            {
                void *record = *_cursor;
                BOOST_CHECK_MESSAGE (record, "Cursor should not be empty.");
                BOOST_REQUIRE_MESSAGE (_task.copyFrom, "New value source must not be null pointer!");

                if (record)
                {
                    memcpy (record, _task.copyFrom, storage.GetRecordMapping ().GetObjectSize ());
                }
            }
            else
            {
                BOOST_REQUIRE_MESSAGE (false, boost::format ("Cursor %1% should be editable.") % _task.name);
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CursorIncrement &_task)
{
    auto iterator = activeCursors.find (_task.name);
    BOOST_REQUIRE_MESSAGE (
        iterator != activeCursors.end (),
        boost::format ("There should be active cursor with name \"%1%\".") % _task.name);

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
    BOOST_REQUIRE_MESSAGE (
        iterator != activeCursors.end (),
        boost::format ("There should be active cursor with name \"%1%\".") % _task.name);

    std::visit (
        [&_task] (auto &_cursor)
        {
            if constexpr (isEditCursor <std::decay_t <decltype (_cursor)>>)
            {
                ~_cursor;
            }
            else
            {
                BOOST_REQUIRE_MESSAGE (false, boost::format ("Cursor %1% should be editable.") % _task.name);
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const CopyCursor &_task)
{
    auto iterator = activeCursors.find (_task.sourceName);
    BOOST_REQUIRE_MESSAGE (
        iterator != activeCursors.end (),
        boost::format ("There should be active cursor with name \"%1%\".") % _task.sourceName);

    BOOST_REQUIRE_MESSAGE (
        activeCursors.find (_task.targetName) == activeCursors.end (),
        boost::format ("There should be no active cursor with name \"%1%\".") % _task.targetName);

    std::visit (
        [this, &_task] (auto &_cursor)
        {
            if constexpr (!isEditCursor <std::decay_t <decltype (_cursor)>>)
            {
                activeCursors.emplace (_task.targetName, _cursor);
            }
            else
            {
                BOOST_REQUIRE_MESSAGE (false,
                                       boost::format ("Cursor %1% should not be editable.") % _task.sourceName);
            }
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const MoveCursor &_task)
{
    auto iterator = activeCursors.find (_task.sourceName);
    BOOST_REQUIRE_MESSAGE (
        iterator != activeCursors.end (),
        boost::format ("There should be active cursor with name \"%1%\".") % _task.sourceName);

    BOOST_REQUIRE_MESSAGE (
        activeCursors.find (_task.targetName) == activeCursors.end (),
        boost::format ("There should be no active cursor with name \"%1%\".") % _task.targetName);

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
    BOOST_REQUIRE_MESSAGE (
        iterator != activeCursors.end (),
        boost::format ("There should be active cursor with name \"%1%\".") % _task.name);

    activeCursors.erase (iterator);
}

const IndexReference &ExecutionContext::PrepareForLookup (const IndexLookupBase &_task) const
{
    BOOST_REQUIRE_MESSAGE (
        activeCursors.find (_task.cursorName) == activeCursors.end (),
        boost::format ("There should be no cursor with name \"%1%\"") % _task.cursorName);

    auto iterator = indexReferences.find (_task.indexName);
    BOOST_REQUIRE_MESSAGE (
        iterator != indexReferences.end (),
        boost::format ("There should be index reference with name \"%1%\".") % _task.indexName);

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

        BOOST_CHECK_EQUAL (found.size (), _known.size ());
        for (const auto &index : found)
        {
            BOOST_CHECK_MESSAGE (std::find (_known.begin (), _known.end (), index) != _known.end (),
                                 boost::format ("Searching received index with address %1% in known list.") %
                                 index);
        }

        for (const auto &index : _known)
        {
            BOOST_CHECK_MESSAGE (std::find (found.begin (), found.end (), index) != found.end (),
                                 boost::format ("Searching known index with address %1% in received list.") %
                                 index);
        }
    };

    BOOST_TEST_MESSAGE ("Checking hash indices.");
    Iterate (storage.BeginHashIndices (), storage.EndHashIndices (), knownHashIndices);

    BOOST_TEST_MESSAGE ("Checking ordered indices.");
    Iterate (storage.BeginOrderedIndices (), storage.EndOrderedIndices (), knownOrderedIndices);

    BOOST_TEST_MESSAGE ("Checking volumetric indices.");
    Iterate (storage.BeginVolumetricIndices (), storage.EndVolumetricIndices (), knownVolumetricIndices);
}

void ExecutionContext::OnIndexDropped (HashIndex *_index)
{
    auto iterator = std::find (knownHashIndices.begin (), knownHashIndices.end (), _index);
    BOOST_CHECK (iterator != knownHashIndices.end ());

    if (iterator != knownHashIndices.end ())
    {
        knownHashIndices.erase (iterator);
    }
}

void ExecutionContext::OnIndexDropped (OrderedIndex *_index)
{
    auto iterator = std::find (knownOrderedIndices.begin (), knownOrderedIndices.end (), _index);
    BOOST_CHECK (iterator != knownOrderedIndices.end ());

    if (iterator != knownOrderedIndices.end ())
    {
        knownOrderedIndices.erase (iterator);
    }
}

void ExecutionContext::OnIndexDropped (VolumetricIndex *_index)
{
    auto iterator = std::find (knownVolumetricIndices.begin (), knownVolumetricIndices.end (), _index);
    BOOST_CHECK (iterator != knownVolumetricIndices.end ());

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

VolumetricIndex::AxisAlignedShape ExecutionContext::ExtractShape (
    const VolumetricIndexShapeIntersectionLookupBase &_task) const
{
    BOOST_REQUIRE_EQUAL (_task.min.size (), _task.max.size ());
    BOOST_REQUIRE_GT (_task.min.size (), 0u);
    BOOST_REQUIRE_LE (_task.min.size (), Constants::VolumetricIndex::MAX_DIMENSIONS);
    VolumetricIndex::AxisAlignedShape shape;

    for (std::size_t index = 0u; index < _task.min.size (); ++index)
    {
        shape.min[index] = *reinterpret_cast<const VolumetricIndex::SupportedAxisValue *> (_task.min[index]);
        shape.max[index] = *reinterpret_cast<const VolumetricIndex::SupportedAxisValue *> (_task.max[index]);
    }

    return shape;
}

VolumetricIndex::Ray ExecutionContext::ExtractRay (const VolumetricIndexRayIntersectionLookupBase &_task) const
{
    BOOST_REQUIRE_EQUAL (_task.origin.size (), _task.direction.size ());
    BOOST_REQUIRE_GT (_task.origin.size (), 0u);
    BOOST_REQUIRE_LE (_task.origin.size (), Constants::VolumetricIndex::MAX_DIMENSIONS);
    VolumetricIndex::Ray ray;

    for (std::size_t index = 0u; index < _task.origin.size (); ++index)
    {
        ray.origin[index] = *reinterpret_cast<const VolumetricIndex::SupportedAxisValue *> (_task.origin[index]);
        ray.direction[index] = *reinterpret_cast<const VolumetricIndex::SupportedAxisValue *> (_task.direction[index]);
    }

    return ray;
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
        _output << " { minField: " << dimension.minBorderField << ", globalMinPointer: " <<
                dimension.globalMinBorderValue << ", maxField: " << dimension.maxBorderField <<
                ", globalMaxPointer: " << dimension.globalMaxBorderValue << " }";
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
            "lookup using shape { minPointers: { ";

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
            "lookup using ray { originPointers: { ";

    for (const void *pointer : _lookup.origin)
    {
        _output << pointer << " ";
    }

    _output << "}, directionPointers: { ";
    for (const void *pointer : _lookup.direction)
    {
        _output << pointer << " ";
    }

    return _output << "} } and save cursor as \"" << _lookup.cursorName << "\".";
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
}

void Scenario::Execute () const
{
    ExecutionContext context (mapping);
    for (const Task &wrappedTask : tasks)
    {
        std::visit (
            [&context] (const auto &_unwrappedTask)
            {
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