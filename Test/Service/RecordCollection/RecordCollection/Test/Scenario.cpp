#include <optional>
#include <sstream>
#include <unordered_map>

#include <Query/Test/CursorManager.hpp>

#include <RecordCollection/Collection.hpp>
#include <RecordCollection/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::RecordCollection::Test
{
using RepresentationReference = std::variant <
    LinearRepresentation,
    PointRepresentation,
    VolumetricRepresentation>;
} // namespace Emergence::RecordCollection::Test

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (
    Emergence::RecordCollection::Test::RepresentationReferenceTag,
    Emergence::RecordCollection::Test::RepresentationReference)

namespace Emergence::RecordCollection::Test
{
struct ExecutionContext final :
    public Context::Extension::ObjectStorage <RepresentationReference>,
    public Query::Test::CursorManager <
        LinearRepresentation::ReadCursor,
        LinearRepresentation::EditCursor,
        LinearRepresentation::ReversedReadCursor,
        LinearRepresentation::ReversedEditCursor,
        PointRepresentation::ReadCursor,
        PointRepresentation::EditCursor,
        VolumetricRepresentation::ShapeIntersectionReadCursor,
        VolumetricRepresentation::ShapeIntersectionEditCursor,
        VolumetricRepresentation::RayIntersectionReadCursor,
        VolumetricRepresentation::RayIntersectionEditCursor>
{
    explicit ExecutionContext (const StandardLayout::Mapping &_typeMapping);

    ~ExecutionContext ();

    Collection collection;
    StandardLayout::Mapping typeMapping;
    std::optional <Collection::Allocator> collectionAllocator;
};

ExecutionContext::ExecutionContext (const StandardLayout::Mapping &_typeMapping)
    : collection (_typeMapping),
      typeMapping (_typeMapping),
      collectionAllocator ()
{
}

ExecutionContext::~ExecutionContext ()
{
    cursors.clear ();
    objects.clear ();
}

std::vector <uint8_t> MergeVectorsIntoRepresentationLookupSequence (
    const VolumetricRepresentation &_representation,
    const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_firstVector,
    const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_secondVector)
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

void IterateOverRepresentations (const ExecutionContext &_context)
{
    std::vector <RepresentationReference> known;
    for (const auto &[name, representation] : _context.objects)
    {
        known.emplace_back (representation);
    }

    std::vector <RepresentationReference> found;
    // During iterations below we will execute some unnecessary operations
    // with iterator to cover more iteration-related operations.

    for (auto iterator = _context.collection.LinearRepresentationBegin ();
         iterator != _context.collection.LinearRepresentationEnd (); ++iterator)
    {
        if (iterator != _context.collection.LinearRepresentationBegin ())
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

    for (auto iterator = _context.collection.PointRepresentationBegin ();
         iterator != _context.collection.PointRepresentationEnd (); ++iterator)
    {
        if (iterator != _context.collection.PointRepresentationBegin ())
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

    for (auto iterator = _context.collection.VolumetricRepresentationBegin ();
         iterator != _context.collection.VolumetricRepresentationEnd (); ++iterator)
    {
        if (iterator != _context.collection.VolumetricRepresentationBegin ())
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

void ExecuteTask (ExecutionContext &_context, const CreatePointRepresentation &_task)
{
    PointRepresentation representation = _context.collection.CreatePointRepresentation (_task.keyFields);
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
            CHECK (_context.typeMapping.GetField (_task.keyFields[keyFieldIndex]).IsSame (*iterator));
            ++keyFieldIndex;
        }
    }

    AddObject <RepresentationReference> (_context, _task.name, representation);
    IterateOverRepresentations (_context);
}

void ExecuteTask (ExecutionContext &_context, const CreateLinearRepresentation &_task)
{
    LinearRepresentation representation = _context.collection.CreateLinearRepresentation (_task.keyField);
    CHECK (_context.typeMapping.GetField (_task.keyField).IsSame (representation.GetKeyField ()));

    AddObject <RepresentationReference> (_context, _task.name, representation);
    IterateOverRepresentations (_context);
}

void ExecuteTask (ExecutionContext &_context, const CreateVolumetricRepresentation &_task)
{
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

    VolumetricRepresentation representation = _context.collection.CreateVolumetricRepresentation (convertedDescriptors);
    // Check that dimensions key fields are equal to expected key fields.
    std::size_t dimensionIndex = 0u;

    for (auto iterator = representation.DimensionBegin (); iterator != representation.DimensionEnd (); ++iterator)
    {
        const bool overflow = dimensionIndex >= _task.dimensions.size ();
        CHECK (!overflow);

        if (!overflow)
        {
            const auto dimension = *iterator;
            CHECK (_context.typeMapping.GetField (
                _task.dimensions[dimensionIndex].minField).IsSame (dimension.minField));

            CHECK (_context.typeMapping.GetField
                (_task.dimensions[dimensionIndex].maxField).IsSame (dimension.maxField));
            ++dimensionIndex;
        }
    }

    AddObject <RepresentationReference> (_context, _task.name, representation);
    IterateOverRepresentations (_context);
}

void ExecuteTask (ExecutionContext &_context, const CheckIsSourceBusy &_task)
{
    std::visit (
        [&_task] (auto &_reference)
        {
            CHECK_EQUAL (_reference.CanBeDropped (), !_task.expectedValue);
        },
        GetObject <RepresentationReference> (_context, _task.name));
}

void ExecuteTask (ExecutionContext &_context, const DropRepresentation &_task)
{
    std::visit (
        [] (auto &_reference)
        {
            _reference.Drop ();
        },
        GetObject <RepresentationReference> (_context, _task.name));

    _context.objects.erase (_context.objects.find (_task.name));
    IterateOverRepresentations (_context);
}

void ExecuteTask (ExecutionContext &_context, const OpenAllocator &)
{
    REQUIRE_WITH_MESSAGE (!_context.collectionAllocator, "There should be no active allocator.");
    _context.collectionAllocator.emplace (_context.collection.AllocateAndInsert ());
}

void ExecuteTask (ExecutionContext &_context, const AllocateAndInit &_task)
{
    REQUIRE_WITH_MESSAGE (_context.collectionAllocator, "There should be active allocator.");
    void *record = _context.collectionAllocator.value ().Allocate ();
    CHECK (record != nullptr);

    if (record)
    {
        memcpy (record, _task.copyFrom, _context.typeMapping.GetObjectSize ());
    }
}

void ExecuteTask (ExecutionContext &_context, const CloseAllocator &)
{
    REQUIRE_WITH_MESSAGE (_context.collectionAllocator, "There should be active allocator.");
    _context.collectionAllocator.reset ();
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToRead &_task)
{
    PointRepresentation representation = std::get <PointRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, _context.typeMapping, representation.ReadPoint (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToEdit &_task)
{
    PointRepresentation representation = std::get <PointRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, _context.typeMapping, representation.EditPoint (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryRangeToRead &_task)
{
    LinearRepresentation representation = std::get <LinearRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));

    AddCursor (_context, _task.cursorName, _context.typeMapping,
               representation.ReadInterval (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryRangeToEdit &_task)
{
    LinearRepresentation representation = std::get <LinearRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));

    AddCursor (_context, _task.cursorName, _context.typeMapping,
               representation.EditInterval (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryReversedRangeToRead &_task)
{
    LinearRepresentation representation = std::get <LinearRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));

    AddCursor (_context, _task.cursorName, _context.typeMapping,
               representation.ReadReversedInterval (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryReversedRangeToEdit &_task)
{
    LinearRepresentation representation = std::get <LinearRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));

    AddCursor (_context, _task.cursorName, _context.typeMapping,
               representation.EditReversedInterval (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToRead &_task)
{
    VolumetricRepresentation representation = std::get <VolumetricRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));

    std::vector <uint8_t> sequence = MergeVectorsIntoRepresentationLookupSequence (
        representation, _task.min, _task.max);

    AddCursor (_context, _task.cursorName, _context.typeMapping,
               representation.ReadShapeIntersections (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToEdit &_task)
{
    VolumetricRepresentation representation = std::get <VolumetricRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));

    std::vector <uint8_t> sequence = MergeVectorsIntoRepresentationLookupSequence (
        representation, _task.min, _task.max);

    AddCursor (_context, _task.cursorName, _context.typeMapping,
               representation.EditShapeIntersections (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToRead &_task)
{
    VolumetricRepresentation representation = std::get <VolumetricRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));

    std::vector <uint8_t> sequence = MergeVectorsIntoRepresentationLookupSequence (
        representation, _task.origin, _task.direction);

    AddCursor (_context, _task.cursorName, _context.typeMapping,
               representation.ReadRayIntersections (&sequence[0u], _task.maxDistance));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToEdit &_task)
{
    VolumetricRepresentation representation = std::get <VolumetricRepresentation> (
        GetObject <RepresentationReference> (_context, _task.sourceName));

    std::vector <uint8_t> sequence = MergeVectorsIntoRepresentationLookupSequence (
        representation, _task.origin, _task.direction);

    AddCursor (_context, _task.cursorName, _context.typeMapping,
               representation.EditRayIntersections (&sequence[0u], _task.maxDistance));
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

std::ostream &operator << (std::ostream &_output, const Copy <RepresentationReferenceTag> &_task)
{
    return _output << "Copy representation reference \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const Delete <RepresentationReferenceTag> &_task)
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
                    using Source = std::decay_t <decltype (_unwrappedSource)>;
                    if constexpr (std::is_same_v <Source, Query::Test::Sources::Value>)
                    {
                        tasks.emplace_back (
                            CreatePointRepresentation {_unwrappedSource.name, _unwrappedSource.queriedFields});
                    }
                    else if constexpr (std::is_same_v <Source, Query::Test::Sources::Range>)
                    {
                        tasks.emplace_back (
                            CreateLinearRepresentation {_unwrappedSource.name, _unwrappedSource.queriedField});
                    }
                    else if constexpr (std::is_same_v <Source, Query::Test::Sources::Volumetric>)
                    {
                        tasks.emplace_back (
                            CreateVolumetricRepresentation {_unwrappedSource.name, _unwrappedSource.dimensions});
                    }
                    else
                    {
                        REQUIRE_WITH_MESSAGE (false, "Only Value, Range and Volumetric sources are supported!");
                    }
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
                using TaskType = std::decay_t <decltype (_unwrappedTask)>;
                if constexpr (std::is_same_v <TaskType, QuerySingletonToRead> ||
                              std::is_same_v <TaskType, QuerySingletonToEdit> ||
                              std::is_same_v <TaskType, QueryUnorderedSequenceToRead> ||
                              std::is_same_v <TaskType, QueryUnorderedSequenceToEdit>)
                {
                    REQUIRE_WITH_MESSAGE (false, "Singleton and unordered sequence queries are not supported!");
                }
                else
                {
                    tasks.emplace_back (_unwrappedTask);
                }
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
                std::stringstream stream;
                stream << _unwrappedTask;
                LOG (stream.str ());
                ExecuteTask (context, _unwrappedTask);
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