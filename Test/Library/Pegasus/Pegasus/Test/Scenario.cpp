#include <optional>
#include <sstream>
#include <unordered_map>

#include <Handling/Handle.hpp>

#include <Pegasus/Storage.hpp>
#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/CursorStorage.hpp>
#include <Query/Test/DataTypes.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Pegasus::Test
{
using IndexReference =
    std::variant<Handling::Handle<HashIndex>, Handling::Handle<OrderedIndex>, Handling::Handle<VolumetricIndex>>;

using Cursor = std::variant<HashIndex::ReadCursor,
                            HashIndex::EditCursor,
                            OrderedIndex::AscendingReadCursor,
                            OrderedIndex::AscendingEditCursor,
                            OrderedIndex::DescendingReadCursor,
                            OrderedIndex::DescendingEditCursor,
                            VolumetricIndex::ShapeIntersectionReadCursor,
                            VolumetricIndex::ShapeIntersectionEditCursor,
                            VolumetricIndex::RayIntersectionReadCursor,
                            VolumetricIndex::RayIntersectionEditCursor>;
} // namespace Emergence::Pegasus::Test

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (Emergence::Pegasus::Test::CursorTag,
                                   Emergence::Query::Test::CursorData<Emergence::Pegasus::Test::Cursor>,
                                   "cursor")

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (Emergence::Pegasus::Test::IndexReferenceTag,
                                   Emergence::Pegasus::Test::IndexReference,
                                   "index reference")

namespace Emergence::Pegasus::Test
{
struct ExecutionContext final : public Context::Extension::ObjectStorage<IndexReference>,
                                public Query::Test::CursorStorage<Cursor>
{
    explicit ExecutionContext (StandardLayout::Mapping _recordMapping);

    ~ExecutionContext ();

    Storage storage;
    std::vector<HashIndex *> knownHashIndices;
    std::vector<OrderedIndex *> knownOrderedIndices;
    std::vector<VolumetricIndex *> knownVolumetricIndices;
    std::optional<Storage::Allocator> storageAllocator;
};

ExecutionContext::ExecutionContext (StandardLayout::Mapping _recordMapping)
    : storage (std::move (_recordMapping)),
      knownHashIndices (),
      knownOrderedIndices (),
      knownVolumetricIndices ()
{
}

ExecutionContext::~ExecutionContext ()
{
    Query::Test::CursorStorage<Cursor>::objects.clear ();
    Context::Extension::ObjectStorage<IndexReference>::objects.clear ();
}

std::vector<std::size_t> CollectVolumetricIndexKeyFieldSizes (const VolumetricIndex *_index)
{
    REQUIRE (_index);
    std::vector<std::size_t> result;

    for (const auto &dimension : _index->GetDimensions ())
    {
        REQUIRE (dimension.minBorderField.GetSize () == dimension.maxBorderField.GetSize ());
        result.emplace_back (dimension.minBorderField.GetSize ());
    }

    return result;
}

void IterateOverIndices (const ExecutionContext &_context)
{
    auto Iterate = [] (auto _current, auto _end, const auto &_known)
    {
        std::decay_t<decltype (_known)> found;
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
    Iterate (_context.storage.BeginHashIndices (), _context.storage.EndHashIndices (), _context.knownHashIndices);

    LOG ("Checking ordered indices.");
    Iterate (_context.storage.BeginOrderedIndices (), _context.storage.EndOrderedIndices (),
             _context.knownOrderedIndices);

    LOG ("Checking volumetric indices.");
    Iterate (_context.storage.BeginVolumetricIndices (), _context.storage.EndVolumetricIndices (),
             _context.knownVolumetricIndices);
}

void OnIndexDropped (ExecutionContext &_context, HashIndex *_index)
{
    auto iterator = std::find (_context.knownHashIndices.begin (), _context.knownHashIndices.end (), _index);
    CHECK (iterator != _context.knownHashIndices.end ());

    if (iterator != _context.knownHashIndices.end ())
    {
        _context.knownHashIndices.erase (iterator);
    }
}

void OnIndexDropped (ExecutionContext &_context, OrderedIndex *_index)
{
    auto iterator = std::find (_context.knownOrderedIndices.begin (), _context.knownOrderedIndices.end (), _index);
    CHECK (iterator != _context.knownOrderedIndices.end ());

    if (iterator != _context.knownOrderedIndices.end ())
    {
        _context.knownOrderedIndices.erase (iterator);
    }
}

void OnIndexDropped (ExecutionContext &_context, VolumetricIndex *_index)
{
    auto iterator =
        std::find (_context.knownVolumetricIndices.begin (), _context.knownVolumetricIndices.end (), _index);
    CHECK (iterator != _context.knownVolumetricIndices.end ());

    if (iterator != _context.knownVolumetricIndices.end ())
    {
        _context.knownVolumetricIndices.erase (iterator);
    }
}

void ExecuteTask (ExecutionContext &_context, const CreateHashIndex &_task)
{
    Handling::Handle<HashIndex> index = _context.storage.CreateHashIndex (_task.indexedFields);
    REQUIRE_WITH_MESSAGE (index, "Returned index should not be null.");

    _context.knownHashIndices.emplace_back (index.Get ());
    AddObject<IndexReference> (_context, _task.name, index);
    IterateOverIndices (_context);
}

void ExecuteTask (ExecutionContext &_context, const CreateOrderedIndex &_task)
{
    Handling::Handle<OrderedIndex> index = _context.storage.CreateOrderedIndex (_task.indexedField);
    REQUIRE_WITH_MESSAGE (index, "Returned index should not be null.");

    _context.knownOrderedIndices.emplace_back (index.Get ());
    AddObject<IndexReference> (_context, _task.name, index);
    IterateOverIndices (_context);
}

void ExecuteTask (ExecutionContext &_context, const CreateVolumetricIndex &_task)
{
    std::vector<VolumetricIndex::DimensionDescriptor> convertedDescriptors;
    convertedDescriptors.reserve (_task.dimensions.size ());

    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _task.dimensions)
    {
        static_assert (sizeof (VolumetricIndex::SupportedAxisValue) == sizeof (dimension.globalMin));

        convertedDescriptors.emplace_back (VolumetricIndex::DimensionDescriptor {
            *reinterpret_cast<const VolumetricIndex::SupportedAxisValue *> (&dimension.globalMin),
            dimension.minField,
            *reinterpret_cast<const VolumetricIndex::SupportedAxisValue *> (&dimension.globalMax),
            dimension.maxField,
        });
    }

    Handling::Handle<VolumetricIndex> index = _context.storage.CreateVolumetricIndex (convertedDescriptors);
    REQUIRE_WITH_MESSAGE (index, "Returned index should not be null.");

    _context.knownVolumetricIndices.emplace_back (index.Get ());
    AddObject<IndexReference> (_context, _task.name, index);
    IterateOverIndices (_context);
}

void ExecuteTask (ExecutionContext &_context, const CheckIsIndexCanBeDropped &_task)
{
    std::visit (
        [&_task] (auto &_handle)
        {
            auto *handleValue = _handle.Get ();
            // Temporary make handle free. Otherwise, CanBeDropped check will always return false.
            _handle = nullptr;
            CHECK_EQUAL (handleValue->CanBeDropped (), _task.expected);
            _handle = handleValue;
        },
        GetObject<IndexReference> (_context, _task.name));
}

void ExecuteTask (ExecutionContext &_context, const DropIndex &_task)
{
    std::visit (
        [&_context] (auto &_handle)
        {
            auto *handleValue = _handle.Get ();
            // Firstly make handle free. Otherwise, CanBeDropped check will always return false.
            _handle = nullptr;

            REQUIRE (handleValue->CanBeDropped ());
            handleValue->Drop ();
            OnIndexDropped (_context, handleValue);
        },
        GetObject<IndexReference> (_context, _task.name));

    ExecuteTask (_context, Delete<IndexReferenceTag> {_task.name});
    IterateOverIndices (_context);
}

void ExecuteTask (ExecutionContext &_context, const OpenAllocator &)
{
    REQUIRE_WITH_MESSAGE (!_context.storageAllocator, "There should be no active allocator.");
    _context.storageAllocator.emplace (_context.storage.AllocateAndInsert ());
}

void ExecuteTask (ExecutionContext &_context, const AllocateAndInit &_task)
{
    REQUIRE_WITH_MESSAGE (_context.storageAllocator, "There should be active allocator.");
    void *record = _context.storageAllocator.value ().Next ();
    CHECK (record != nullptr);

    if (record)
    {
        memcpy (record, _task.copyFrom, _context.storage.GetRecordMapping ().GetObjectSize ());
    }
}

void ExecuteTask (ExecutionContext &_context, const CloseAllocator &)
{
    REQUIRE_WITH_MESSAGE (_context.storageAllocator, "There should be active allocator.");
    _context.storageAllocator.reset ();
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToRead &_task)
{
    HashIndex *index =
        std::get<Handling::Handle<HashIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupToRead ({_task.value}));
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToEdit &_task)
{
    HashIndex *index =
        std::get<Handling::Handle<HashIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupToEdit ({_task.value}));
}

void ExecuteTask (ExecutionContext &_context, const QueryAscendingRangeToRead &_task)
{
    OrderedIndex *index =
        std::get<Handling::Handle<OrderedIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupToReadAscending ({_task.minValue}, {_task.maxValue}));
}

void ExecuteTask (ExecutionContext &_context, const QueryAscendingRangeToEdit &_task)
{
    OrderedIndex *index =
        std::get<Handling::Handle<OrderedIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupToEditAscending ({_task.minValue}, {_task.maxValue}));
}

void ExecuteTask (ExecutionContext &_context, const QueryDescendingRangeToRead &_task)
{
    OrderedIndex *index =
        std::get<Handling::Handle<OrderedIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupToReadDescending ({_task.minValue}, {_task.maxValue}));
}

void ExecuteTask (ExecutionContext &_context, const QueryDescendingRangeToEdit &_task)
{
    OrderedIndex *index =
        std::get<Handling::Handle<OrderedIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupToEditDescending ({_task.minValue}, {_task.maxValue}));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToRead &_task)
{
    VolumetricIndex *index =
        std::get<Handling::Handle<VolumetricIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    std::vector<uint8_t> sequence =
        Query::Test::LayoutShapeIntersectionQueryParameters (_task, CollectVolumetricIndexKeyFieldSizes (index));

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupShapeIntersectionToRead (
                           *reinterpret_cast<VolumetricIndex::AxisAlignedShapeContainer *> (&sequence[0u])));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToEdit &_task)
{
    VolumetricIndex *index =
        std::get<Handling::Handle<VolumetricIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    std::vector<uint8_t> sequence =
        Query::Test::LayoutShapeIntersectionQueryParameters (_task, CollectVolumetricIndexKeyFieldSizes (index));

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupShapeIntersectionToEdit (
                           *reinterpret_cast<VolumetricIndex::AxisAlignedShapeContainer *> (&sequence[0u])));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToRead &_task)
{
    VolumetricIndex *index =
        std::get<Handling::Handle<VolumetricIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    std::vector<uint8_t> sequence =
        Query::Test::LayoutRayIntersectionQueryParameters (_task, CollectVolumetricIndexKeyFieldSizes (index));

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupRayIntersectionToRead (
                           *reinterpret_cast<VolumetricIndex::RayContainer *> (&sequence[0u]), _task.maxDistance));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToEdit &_task)
{
    VolumetricIndex *index =
        std::get<Handling::Handle<VolumetricIndex>> (GetObject<IndexReference> (_context, _task.sourceName)).Get ();

    std::vector<uint8_t> sequence =
        Query::Test::LayoutRayIntersectionQueryParameters (_task, CollectVolumetricIndexKeyFieldSizes (index));

    AddObject<Cursor> (_context, _task.cursorName, _context.storage.GetRecordMapping (),
                       index->LookupRayIntersectionToEdit (
                           *reinterpret_cast<VolumetricIndex::RayContainer *> (&sequence[0u]), _task.maxDistance));
}

std::ostream &operator<< (std::ostream &_output, const CreateHashIndex &_task)
{
    _output << "Create hash index \"" << _task.name << "\" on fields:";
    for (StandardLayout::FieldId fieldId : _task.indexedFields)
    {
        _output << " " << fieldId;
    }

    return _output << ".";
}

std::ostream &operator<< (std::ostream &_output, const CreateOrderedIndex &_task)
{
    return _output << "Create ordered index \"" << _task.name << "\" on field " << _task.indexedField << ".";
}

std::ostream &operator<< (std::ostream &_output, const CreateVolumetricIndex &_task)
{
    _output << "Create volumetric index \"" << _task.name << "\" on dimensions:";
    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _task.dimensions)
    {
        _output << " { globalMin: " << dimension.globalMin << ", minField: " << dimension.minField
                << ", globalMax: " << dimension.globalMax << ", maxField: " << dimension.maxField << " }";
    }

    return _output << ".";
}

std::ostream &operator<< (std::ostream &_output, const CheckIsIndexCanBeDropped &_task)
{
    return _output << "Check is index \"" << _task.name << "\" can be dropped, expected result: \""
                   << (_task.expected ? "yes" : "no") << "\".";
}

std::ostream &operator<< (std::ostream &_output, const DropIndex &_task)
{
    return _output << "Drop index \"" << _task.name << "\".";
}

std::ostream &operator<< (std::ostream &_output, const OpenAllocator &)
{
    return _output << "Open allocator.";
}

std::ostream &operator<< (std::ostream &_output, const AllocateAndInit &_task)
{
    return _output << "Allocate record and init from " << _task.copyFrom << ".";
}

std::ostream &operator<< (std::ostream &_output, const CloseAllocator &)
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

std::vector<Task> CreateIndices (const Query::Test::Storage &_storage)
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
                    tasks.emplace_back (CreateHashIndex {_source.name, _source.queriedFields});
                }
                else if constexpr (std::is_same_v<Source, Query::Test::Sources::Range>)
                {
                    tasks.emplace_back (CreateOrderedIndex {_source.name, _source.queriedField});
                }
                else if constexpr (std::is_same_v<Source, Query::Test::Sources::Volumetric>)
                {
                    tasks.emplace_back (CreateVolumetricIndex {_source.name, _source.dimensions});
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

static void ExecuteScenario (const Query::Test::Scenario &_scenario, bool _insertFirst)
{
    std::vector<Task> tasks;
    REQUIRE_WITH_MESSAGE (_scenario.storages.size () == 1u,
                          "Only one-storage tests are supported right now, because Pegasus storages are independent.");

    if (_insertFirst)
    {
        tasks += InsertRecords (_scenario.storages[0u]);
        tasks += CreateIndices (_scenario.storages[0u]);
    }
    else
    {
        tasks += CreateIndices (_scenario.storages[0u]);
        tasks += InsertRecords (_scenario.storages[0u]);
    }

    for (const Query::Test::Task &task : _scenario.tasks)
    {
        tasks.emplace_back (ImportTask (task));
    }

    Scenario (_scenario.storages[0u].dataType, tasks);
}

void CreateIndicesThanInsertRecords (const Query::Test::Scenario &_scenario)
{
    ExecuteScenario (_scenario, false);
}

void InsertRecordsThanCreateIndices (const Query::Test::Scenario &_scenario)
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

void ForIndexReference (const Reference::Test::Scenario &_scenario, const Query::Test::Storage &_storage)
{
    REQUIRE (_storage.sources.size () == 1u);
    const std::string indexName = ExtractSourceName (_storage.sources[0u]);
    std::vector<Task> tasks = TestQueryApiDrivers::CreateIndices (_storage);
    tasks += TestQueryApiDrivers::InsertRecords (_storage);

    for (const Reference::Test::Task &packedTask : _scenario)
    {
        std::visit (
            [&tasks, &indexName] (const auto &_task)
            {
                using TaskType = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Create>)
                {
                    tasks.emplace_back (Copy<IndexReferenceTag> {indexName, _task.name});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Move>)
                {
                    tasks.emplace_back (Move<IndexReferenceTag> {_task.sourceName, _task.targetName});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Copy>)
                {
                    tasks.emplace_back (Copy<IndexReferenceTag> {_task.sourceName, _task.targetName});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::MoveAssign>)
                {
                    tasks.emplace_back (MoveAssign<IndexReferenceTag> {_task.sourceName, _task.targetName});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::CopyAssign>)
                {
                    tasks.emplace_back (CopyAssign<IndexReferenceTag> {_task.sourceName, _task.targetName});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Delete>)
                {
                    tasks.emplace_back (Delete<IndexReferenceTag> {_task.name});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::CheckStatus>)
                {
                    tasks.emplace_back (CheckIsIndexCanBeDropped {indexName, !_task.hasAnyReferences});
                }
                else
                {
                    REQUIRE_WITH_MESSAGE (false, "Unknown task type!");
                }
            },
            packedTask);
    }

    tasks.emplace_back (DropIndex {indexName});
    Scenario {_storage.dataType, tasks};
}

void ForCursor (const Reference::Test::Scenario &_scenario,
                const Query::Test::Storage &_storage,
                const Query::Test::Task &_sourceQuery,
                const void *_expectedPointedObject)
{
    REQUIRE (_storage.sources.size () == 1u);
    const std::string indexName = ExtractSourceName (_storage.sources[0u]);
    std::vector<Task> tasks = TestQueryApiDrivers::CreateIndices (_storage);
    tasks += TestQueryApiDrivers::InsertRecords (_storage);

    for (const Reference::Test::Task &packedTask : _scenario)
    {
        std::visit (
            [&tasks, &indexName, &_sourceQuery, _expectedPointedObject] (const auto &_task)
            {
                using TaskType = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Create>)
                {
                    tasks.emplace_back (TestQueryApiDrivers::ImportTask (
                        Query::Test::ChangeQuerySourceAndCursor (_sourceQuery, indexName, _task.name)));
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
                    tasks.emplace_back (CheckIsIndexCanBeDropped {indexName, !_task.hasAnyReferences});
                }
                else
                {
                    REQUIRE_WITH_MESSAGE (false, "Unknown task type!");
                }
            },
            packedTask);
    }

    tasks.emplace_back (DropIndex {indexName});
    Scenario {_storage.dataType, tasks};
}
} // namespace ReferenceApiTestImporters

Scenario::Scenario (StandardLayout::Mapping _mapping, std::vector<Task> _tasks)
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
                ExecuteTask (context, _unwrappedTask);
            },
            wrappedTask);
    }
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

std::vector<Task> &operator+= (std::vector<Task> &first, const std::vector<Task> &second) noexcept
{
    first.insert (first.end (), second.begin (), second.end ());
    return first;
}
} // namespace Emergence::Pegasus::Test
