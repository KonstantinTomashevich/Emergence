#include <cstring>
#include <optional>
#include <sstream>
#include <unordered_map>

#include <Query/Test/CursorStorage.hpp>
#include <Query/Test/DataTypes.hpp>

#include <RecordCollection/Collection.hpp>
#include <RecordCollection/Test/Scenario.hpp>
#include <RecordCollection/Visualization.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::RecordCollection::Test
{
using RepresentationReference = std::variant<LinearRepresentation, PointRepresentation, VolumetricRepresentation>;

using Cursor = std::variant<LinearRepresentation::AscendingReadCursor,
                            LinearRepresentation::AscendingEditCursor,
                            LinearRepresentation::DescendingReadCursor,
                            LinearRepresentation::DescendingEditCursor,
                            PointRepresentation::ReadCursor,
                            PointRepresentation::EditCursor,
                            VolumetricRepresentation::ShapeIntersectionReadCursor,
                            VolumetricRepresentation::ShapeIntersectionEditCursor,
                            VolumetricRepresentation::RayIntersectionReadCursor,
                            VolumetricRepresentation::RayIntersectionEditCursor>;
} // namespace Emergence::RecordCollection::Test

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (Emergence::RecordCollection::Test::CursorTag,
                                   Emergence::Query::Test::CursorData<Emergence::RecordCollection::Test::Cursor>,
                                   "cursor")

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (Emergence::RecordCollection::Test::RepresentationReferenceTag,
                                   Emergence::RecordCollection::Test::RepresentationReference,
                                   "representation reference")

namespace Emergence::RecordCollection::Test
{
struct ExecutionContext final : public Context::Extension::ObjectStorage<RepresentationReference>,
                                public Query::Test::CursorStorage<Cursor>
{
    explicit ExecutionContext (const StandardLayout::Mapping &_typeMapping);

    ExecutionContext (const ExecutionContext &_other) = delete;

    ExecutionContext (ExecutionContext &&_other) = delete;

    ~ExecutionContext ();

    ExecutionContext &operator= (const ExecutionContext &_other) = delete;

    ExecutionContext &operator= (ExecutionContext &&_other) = delete;

    Collection collection;
    std::optional<Collection::Allocator> collectionAllocator;
};

ExecutionContext::ExecutionContext (const StandardLayout::Mapping &_typeMapping) : collection (_typeMapping)

{
}

ExecutionContext::~ExecutionContext ()
{
    Query::Test::CursorStorage<Cursor>::objects.clear ();
    Context::Extension::ObjectStorage<RepresentationReference>::objects.clear ();
}

std::vector<std::size_t> CollectVolumetricRepresentationKeyFieldSizes (const VolumetricRepresentation &_representation)
{
    std::vector<std::size_t> result;
    for (auto iterator = _representation.DimensionBegin (); iterator != _representation.DimensionEnd (); ++iterator)
    {
        auto dimension = *iterator;
        REQUIRE (dimension.minField.GetSize () == dimension.maxField.GetSize ());
        result.emplace_back (dimension.minField.GetSize ());
    }

    return result;
}

void IterateOverRepresentations (const ExecutionContext &_context)
{
    std::vector<RepresentationReference> known;
    for (const auto &[name, representation] :
         _context.Context::Extension::ObjectStorage<RepresentationReference>::objects)
    {
        // TODO: Some references can be moved out, therefore we must skip them.
        //       Current iteration test routine should be refactored, therefore this hack is ok for now.
        if (*reinterpret_cast<const void *const *> (&representation))
        {
            known.emplace_back (representation);
        }
    }

    std::vector<RepresentationReference> found;
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
                            *reinterpret_cast<const void *const *> (&representation), " in known list.");
    }

    for (const auto &representation : known)
    {
        CHECK_WITH_MESSAGE (std::find (found.begin (), found.end (), representation) != found.end (),
                            "Searching known representation with address ",
                            *reinterpret_cast<const void *const *> (&representation), " in received list.");
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
            CHECK (_context.collection.GetTypeMapping ().GetField (_task.keyFields[keyFieldIndex]).IsSame (*iterator));
            ++keyFieldIndex;
        }
    }

    AddObject<RepresentationReference> (_context, _task.name, representation);
    IterateOverRepresentations (_context);
}

void ExecuteTask (ExecutionContext &_context, const CreateLinearRepresentation &_task)
{
    LinearRepresentation representation = _context.collection.CreateLinearRepresentation (_task.keyField);
    CHECK (_context.collection.GetTypeMapping ().GetField (_task.keyField).IsSame (representation.GetKeyField ()));

    AddObject<RepresentationReference> (_context, _task.name, representation);
    IterateOverRepresentations (_context);
}

void ExecuteTask (ExecutionContext &_context, const CreateVolumetricRepresentation &_task)
{
    std::vector<Collection::DimensionDescriptor> convertedDescriptors;
    convertedDescriptors.reserve (_task.dimensions.size ());

    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _task.dimensions)
    {
        convertedDescriptors.emplace_back (Collection::DimensionDescriptor {
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
            CHECK (_context.collection.GetTypeMapping ()
                       .GetField (_task.dimensions[dimensionIndex].minField)
                       .IsSame (dimension.minField));

            CHECK (_context.collection.GetTypeMapping ()
                       .GetField (_task.dimensions[dimensionIndex].maxField)
                       .IsSame (dimension.maxField));
            ++dimensionIndex;
        }
    }

    AddObject<RepresentationReference> (_context, _task.name, representation);
    IterateOverRepresentations (_context);
}

void ExecuteTask (ExecutionContext &_context, const CheckIsRepresentationCanBeDropped &_task)
{
    std::visit (
        [&_task] (auto &_reference)
        {
            CHECK_EQUAL (_reference.CanBeDropped (), _task.expected);
        },
        GetObject<RepresentationReference> (_context, _task.name));
}

void ExecuteTask (ExecutionContext &_context, const DropRepresentation &_task)
{
    std::visit (
        [] (auto &_reference)
        {
            _reference.Drop ();
        },
        GetObject<RepresentationReference> (_context, _task.name));

    ExecuteTask (_context, Delete<RepresentationReferenceTag> {_task.name});
    IterateOverRepresentations (_context);
}

void ExecuteTask (ExecutionContext &_context, const OpenAllocator & /*unused*/)
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
        memcpy (record, _task.copyFrom, _context.collection.GetTypeMapping ().GetObjectSize ());
    }
}

void ExecuteTask (ExecutionContext &_context, const CloseAllocator & /*unused*/)
{
    REQUIRE_WITH_MESSAGE (_context.collectionAllocator, "There should be active allocator.");
    _context.collectionAllocator.reset ();
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToRead &_task)
{
    PointRepresentation representation =
        std::get<PointRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.ReadPoint (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToEdit &_task)
{
    PointRepresentation representation =
        std::get<PointRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.EditPoint (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryAscendingRangeToRead &_task)
{
    LinearRepresentation representation =
        std::get<LinearRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));

    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.ReadAscendingInterval (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryAscendingRangeToEdit &_task)
{
    LinearRepresentation representation =
        std::get<LinearRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));

    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.EditAscendingInterval (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryDescendingRangeToRead &_task)
{
    LinearRepresentation representation =
        std::get<LinearRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));

    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.ReadDescendingInterval (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryDescendingRangeToEdit &_task)
{
    LinearRepresentation representation =
        std::get<LinearRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));

    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.EditDescendingInterval (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToRead &_task)
{
    VolumetricRepresentation representation =
        std::get<VolumetricRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));

    std::vector<uint8_t> sequence = Query::Test::LayoutShapeIntersectionQueryParameters (
        _task, CollectVolumetricRepresentationKeyFieldSizes (representation));

    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.ReadShapeIntersections (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToEdit &_task)
{
    VolumetricRepresentation representation =
        std::get<VolumetricRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));

    std::vector<uint8_t> sequence = Query::Test::LayoutShapeIntersectionQueryParameters (
        _task, CollectVolumetricRepresentationKeyFieldSizes (representation));

    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.EditShapeIntersections (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToRead &_task)
{
    VolumetricRepresentation representation =
        std::get<VolumetricRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));

    std::vector<uint8_t> sequence = Query::Test::LayoutRayIntersectionQueryParameters (
        _task, CollectVolumetricRepresentationKeyFieldSizes (representation));

    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.ReadRayIntersections (&sequence[0u], _task.maxDistance));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToEdit &_task)
{
    VolumetricRepresentation representation =
        std::get<VolumetricRepresentation> (GetObject<RepresentationReference> (_context, _task.sourceName));

    std::vector<uint8_t> sequence = Query::Test::LayoutRayIntersectionQueryParameters (
        _task, CollectVolumetricRepresentationKeyFieldSizes (representation));

    AddObject<Cursor> (_context, _task.cursorName, _context.collection.GetTypeMapping (),
                       representation.EditRayIntersections (&sequence[0u], _task.maxDistance));
}

std::ostream &operator<< (std::ostream &_output, const CreateLinearRepresentation &_task)
{
    return _output << "Create linear representation \"" << _task.name << "\" on field " << _task.keyField << ".";
}

std::ostream &operator<< (std::ostream &_output, const CreatePointRepresentation &_task)
{
    _output << "Create point representation \"" << _task.name << "\" on fields:";
    for (StandardLayout::FieldId fieldId : _task.keyFields)
    {
        _output << " " << fieldId;
    }

    return _output << ".";
}

std::ostream &operator<< (std::ostream &_output, const CreateVolumetricRepresentation &_task)
{
    _output << "Create volumetric representation \"" << _task.name << "\" on dimensions:";
    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _task.dimensions)
    {
        _output << " { globalMin: " << dimension.globalMin << ", minField: " << dimension.minField
                << ", globalMax: " << dimension.globalMax << ", maxField: " << dimension.maxField << " }";
    }

    return _output << ".";
}

std::ostream &operator<< (std::ostream &_output, const CheckIsRepresentationCanBeDropped &_task)
{
    return _output << "Check is representation \"" << _task.name << "\" can be dropped, expected result: \""
                   << (_task.expected ? "yes" : "no") << "\".";
}

std::ostream &operator<< (std::ostream &_output, const DropRepresentation &_task)
{
    return _output << "Drop representation \"" << _task.name << "\".";
}

std::ostream &operator<< (std::ostream &_output, const OpenAllocator & /*unused*/)
{
    return _output << "Open allocator.";
}

std::ostream &operator<< (std::ostream &_output, const AllocateAndInit &_task)
{
    return _output << "Allocate record and init from " << _task.copyFrom << ".";
}

std::ostream &operator<< (std::ostream &_output, const CloseAllocator & /*unused*/)
{
    return _output << "Close allocator.";
}

namespace TestQueryApiDrivers
{
Task ImportTask (const Query::Test::Task &_task)
{
    return std::visit (
        [] (const auto &_unwrappedTask) -> Task
        {
            using TaskType = std::decay_t<decltype (_unwrappedTask)>;
            if constexpr (std::is_same_v<TaskType, QuerySingletonToRead> ||
                          std::is_same_v<TaskType, QuerySingletonToEdit> ||
                          std::is_same_v<TaskType, QueryUnorderedSequenceToRead> ||
                          std::is_same_v<TaskType, QueryUnorderedSequenceToEdit>)
            {
                REQUIRE_WITH_MESSAGE (false, "Singleton and unordered sequence queries are not supported!");
                // Will never be reached because of REQUIRE above. Added to suppress no-return-value warning.
                throw std::runtime_error ("Singleton and unordered sequence queries are not supported!");
            }
            else
            {
                return _unwrappedTask;
            }
        },
        _task);
}

std::vector<Task> InsertRecords (const Query::Test::Storage &_storage)
{
    std::vector<Task> tasks {OpenAllocator {}};
    for (const void *record : _storage.objectsToInsert)
    {
        tasks.emplace_back (AllocateAndInit {record});
    }

    tasks.emplace_back (CloseAllocator {});
    return tasks;
}

std::vector<Task> CreateRepresentations (const Query::Test::Storage &_storage)
{
    std::vector<Task> tasks;
    for (const Query::Test::Source &source : _storage.sources)
    {
        std::visit (
            [&tasks] (const auto &_source)
            {
                using Source = std::decay_t<decltype (_source)>;
                if constexpr (std::is_same_v<Source, Query::Test::Sources::Value>)
                {
                    tasks.emplace_back (CreatePointRepresentation {_source.name, _source.queriedFields});
                }
                else if constexpr (std::is_same_v<Source, Query::Test::Sources::Range>)
                {
                    tasks.emplace_back (CreateLinearRepresentation {_source.name, _source.queriedField});
                }
                else if constexpr (std::is_same_v<Source, Query::Test::Sources::Volumetric>)
                {
                    tasks.emplace_back (CreateVolumetricRepresentation {_source.name, _source.dimensions});
                }
                else
                {
                    REQUIRE_WITH_MESSAGE (false, "Only Value, Range and Volumetric sources are supported!");
                }
            },
            source);
    }

    return tasks;
}

static void ExecuteScenario (const Query::Test::Scenario &_scenario, bool _allocateFirst)
{
    std::vector<Task> tasks;
    REQUIRE_WITH_MESSAGE (
        _scenario.storages.size () == 1u,
        "Only one-storage tests are supported right now, because record collections are independent.");

    if (_allocateFirst)
    {
        tasks += InsertRecords (_scenario.storages[0u]);
        tasks += CreateRepresentations (_scenario.storages[0u]);
    }
    else
    {
        tasks += CreateRepresentations (_scenario.storages[0u]);
        tasks += InsertRecords (_scenario.storages[0u]);
    }

    for (const Query::Test::Task &task : _scenario.tasks)
    {
        tasks.emplace_back (ImportTask (task));
    }

    Scenario {_scenario.storages[0u].dataType, tasks}.Execute ();
}

void CreateRepresentationsThanAllocateRecords (const Query::Test::Scenario &_scenario)
{
    ExecuteScenario (_scenario, false);
}

void AllocateRecordsThanCreateRepresentations (const Query::Test::Scenario &_scenario)
{
    ExecuteScenario (_scenario, true);
}
} // namespace TestQueryApiDrivers

namespace ReferenceApiTestImporters
{
std::string ExtractSourceName (const Query::Test::Source &_source)
{
    return std::visit (
        [] (const auto &_unwrappedSource)
        {
            return _unwrappedSource.name;
        },
        _source);
}

void ForRepresentationReference (const Reference::Test::Scenario &_scenario, const Query::Test::Storage &_storage)
{
    REQUIRE (_storage.sources.size () == 1u);
    const std::string representationName = ExtractSourceName (_storage.sources[0u]);
    std::vector<Task> tasks = TestQueryApiDrivers::CreateRepresentations (_storage);
    tasks += TestQueryApiDrivers::InsertRecords (_storage);

    for (const Reference::Test::Task &packedTask : _scenario)
    {
        std::visit (
            [&tasks, &representationName] (const auto &_task)
            {
                using TaskType = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Create>)
                {
                    tasks.emplace_back (Copy<RepresentationReferenceTag> {representationName, _task.name});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Move>)
                {
                    tasks.emplace_back (Move<RepresentationReferenceTag> {_task.sourceName, _task.targetName});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Copy>)
                {
                    tasks.emplace_back (Copy<RepresentationReferenceTag> {_task.sourceName, _task.targetName});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::MoveAssign>)
                {
                    tasks.emplace_back (MoveAssign<RepresentationReferenceTag> {_task.sourceName, _task.targetName});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::CopyAssign>)
                {
                    tasks.emplace_back (CopyAssign<RepresentationReferenceTag> {_task.sourceName, _task.targetName});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Delete>)
                {
                    tasks.emplace_back (Delete<RepresentationReferenceTag> {_task.name});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::CheckStatus>)
                {
                    tasks.emplace_back (
                        CheckIsRepresentationCanBeDropped {representationName, !_task.hasAnyReferences});
                }
                else
                {
                    REQUIRE_WITH_MESSAGE (false, "Unknown task type!");
                }
            },
            packedTask);
    }

    tasks.emplace_back (DropRepresentation {representationName});
    Scenario {_storage.dataType, tasks}.Execute ();
}

void ForCursor (const Reference::Test::Scenario &_scenario,
                const Query::Test::Storage &_storage,
                const Query::Test::Task &_sourceQuery,
                const void *_expectedPointedObject)
{
    REQUIRE (_storage.sources.size () == 1u);
    const std::string representationName = ExtractSourceName (_storage.sources[0u]);
    std::vector<Task> tasks = TestQueryApiDrivers::CreateRepresentations (_storage);
    tasks += TestQueryApiDrivers::InsertRecords (_storage);

    for (const Reference::Test::Task &packedTask : _scenario)
    {
        std::visit (
            [&tasks, &representationName, &_sourceQuery, _expectedPointedObject] (const auto &_task)
            {
                using TaskType = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Create>)
                {
                    tasks.emplace_back (TestQueryApiDrivers::ImportTask (
                        Query::Test::ChangeQuerySourceAndCursor (_sourceQuery, representationName, _task.name)));
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Move>)
                {
                    tasks.emplace_back (Move<CursorTag> {_task.sourceName, _task.targetName});
                    tasks.emplace_back (CursorCheck {_task.targetName, _expectedPointedObject});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Copy>)
                {
                    tasks.emplace_back (Copy<CursorTag> {_task.sourceName, _task.targetName});
                    tasks.emplace_back (CursorCheck {_task.targetName, _expectedPointedObject});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Delete>)
                {
                    tasks.emplace_back (Delete<CursorTag> {_task.name});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::CheckStatus>)
                {
                    tasks.emplace_back (
                        CheckIsRepresentationCanBeDropped {representationName, !_task.hasAnyReferences});
                }
                else
                {
                    REQUIRE_WITH_MESSAGE (false, "Unknown task type!");
                }
            },
            packedTask);
    }

    tasks.emplace_back (DropRepresentation {representationName});
    Scenario {_storage.dataType, tasks}.Execute ();
}
} // namespace ReferenceApiTestImporters

static void ExecuteScenario (const Scenario &_scenario, VisualGraph::Graph *_graphOutput)
{
    ExecutionContext context {_scenario.mapping};
    LOG ((std::stringstream () << _scenario).str ());

    for (const Task &wrappedTask : _scenario.tasks)
    {
        std::visit (
            [&context] (const auto &_unwrappedTask)
            {
                LOG ((std::stringstream () << _unwrappedTask).str ());
                ExecuteTask (context, _unwrappedTask);
            },
            wrappedTask);
    }

    if (_graphOutput)
    {
        *_graphOutput = Visualization::GraphFromCollection (context.collection);
    }
}

void Scenario::Execute () const
{
    ExecuteScenario (*this, nullptr);
}

VisualGraph::Graph Scenario::ExecuteAndVisualize () const
{
    VisualGraph::Graph graph;
    ExecuteScenario (*this, &graph);
    return graph;
}

std::ostream &operator<< (std::ostream &_output, const Scenario &_scenario)
{
    _output << "Scenario (on mapping \"" << _scenario.mapping.GetName () << "\"):" << std::endl;
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

std::vector<Task> &operator+= (std::vector<Task> &_first, const std::vector<Task> &_second)
{
    _first.insert (_first.end (), _second.begin (), _second.end ());
    return _first;
}
} // namespace Emergence::RecordCollection::Test
