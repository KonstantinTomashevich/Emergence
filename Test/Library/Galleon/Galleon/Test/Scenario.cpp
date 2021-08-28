#include <optional>
#include <sstream>
#include <unordered_map>

#include <Galleon/CargoDeck.hpp>
#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/CursorStorage.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Galleon::Test
{
using ContainerReference = std::variant<Handling::Handle<SingletonContainer>,
                                        Handling::Handle<ShortTermContainer>,
                                        Handling::Handle<LongTermContainer>>;

using PreparedQuery = std::variant<SingletonContainer::FetchQuery,
                                   SingletonContainer::ModifyQuery,
                                   ShortTermContainer::InsertQuery,
                                   ShortTermContainer::FetchQuery,
                                   ShortTermContainer::ModifyQuery,
                                   LongTermContainer::InsertQuery,
                                   LongTermContainer::FetchValueQuery,
                                   LongTermContainer::ModifyValueQuery,
                                   LongTermContainer::FetchAscendingRangeQuery,
                                   LongTermContainer::ModifyAscendingRangeQuery,
                                   LongTermContainer::FetchDescendingRangeQuery,
                                   LongTermContainer::ModifyDescendingRangeQuery,
                                   LongTermContainer::FetchShapeIntersectionQuery,
                                   LongTermContainer::ModifyShapeIntersectionQuery,
                                   LongTermContainer::FetchRayIntersectionQuery,
                                   LongTermContainer::ModifyRayIntersectionQuery>;

using Cursor = std::variant<SingletonContainer::FetchQuery::Cursor,
                            SingletonContainer::ModifyQuery::Cursor,
                            ShortTermContainer::FetchQuery::Cursor,
                            ShortTermContainer::ModifyQuery::Cursor,
                            LongTermContainer::FetchValueQuery::Cursor,
                            LongTermContainer::ModifyValueQuery::Cursor,
                            LongTermContainer::FetchAscendingRangeQuery::Cursor,
                            LongTermContainer::ModifyAscendingRangeQuery::Cursor,
                            LongTermContainer::FetchDescendingRangeQuery::Cursor,
                            LongTermContainer::ModifyDescendingRangeQuery::Cursor,
                            LongTermContainer::FetchShapeIntersectionQuery::Cursor,
                            LongTermContainer::ModifyShapeIntersectionQuery::Cursor,
                            LongTermContainer::FetchRayIntersectionQuery::Cursor,
                            LongTermContainer::ModifyRayIntersectionQuery::Cursor>;
} // namespace Emergence::Galleon::Test

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (Emergence::Galleon::Test::ContainerReferenceTag,
                                   Emergence::Galleon::Test::ContainerReference,
                                   "container reference")

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (Emergence::Galleon::Test::CursorTag,
                                   Emergence::Query::Test::CursorData<Emergence::Galleon::Test::Cursor>,
                                   "cursor")

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (Emergence::Galleon::Test::PreparedQueryTag,
                                   Emergence::Galleon::Test::PreparedQuery,
                                   "prepared query")

namespace Emergence::Galleon::Test
{
struct ExecutionContext final : public Context::Extension::ObjectStorage<ContainerReference>,
                                public Context::Extension::ObjectStorage<PreparedQuery>,
                                public Query::Test::CursorStorage<Cursor>
{
    ExecutionContext () = default;

    ~ExecutionContext ();

    CargoDeck deck;
};

ExecutionContext::~ExecutionContext ()
{
    Query::Test::CursorStorage<Cursor>::objects.clear ();
    Context::Extension::ObjectStorage<ContainerReference>::objects.clear ();
    Context::Extension::ObjectStorage<PreparedQuery>::objects.clear ();
}

std::vector<RecordCollection::Collection::DimensionDescriptor> ConvertDimensions (
    const std::vector<Query::Test::Sources::Volumetric::Dimension> &_dimensions)
{
    std::vector<RecordCollection::Collection::DimensionDescriptor> convertedDimensions;
    convertedDimensions.reserve (_dimensions.size ());

    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _dimensions)
    {
        convertedDimensions.emplace_back (RecordCollection::Collection::DimensionDescriptor {
            &dimension.globalMin,
            dimension.minField,
            &dimension.globalMax,
            dimension.maxField,
        });
    }

    return convertedDimensions;
}

template <typename QueryType>
std::vector<std::size_t> CollectVolumetricQueryKeyFieldSizes (const QueryType &_query)
{
    std::vector<std::size_t> result;
    for (auto iterator = _query.DimensionBegin (); iterator != _query.DimensionEnd (); ++iterator)
    {
        auto dimension = *iterator;
        REQUIRE (dimension.minField.GetSize () == dimension.maxField.GetSize ());
        result.emplace_back (dimension.minField.GetSize ());
    }

    return result;
}

void ExecuteTask (ExecutionContext &_context, const AcquireSingletonContainer &_task)
{
    AddObject<ContainerReference> (_context, _task.name, _context.deck.AcquireSingletonContainer (_task.mapping));
}

void ExecuteTask (ExecutionContext &_context, const AcquireShortTermContainer &_task)
{
    AddObject<ContainerReference> (_context, _task.name, _context.deck.AcquireShortTermContainer (_task.mapping));
}

void ExecuteTask (ExecutionContext &_context, const AcquireLongTermContainer &_task)
{
    AddObject<ContainerReference> (_context, _task.name, _context.deck.AcquireLongTermContainer (_task.mapping));
}

void ExecuteTask (ExecutionContext &_context, const CheckIsSingletonContainerAllocated &_task)
{
    CHECK_EQUAL (_context.deck.IsSingletonContainerAllocated (_task.mapping), _task.expected);
}

void ExecuteTask (ExecutionContext &_context, const CheckIsShortTermContainerAllocated &_task)
{
    CHECK_EQUAL (_context.deck.IsShortTermContainerAllocated (_task.mapping), _task.expected);
}

void ExecuteTask (ExecutionContext &_context, const CheckIsLongTermContainerAllocated &_task)
{
    CHECK_EQUAL (_context.deck.IsLongTermContainerAllocated (_task.mapping), _task.expected);
}

void ExecuteTask (ExecutionContext &_context, const PrepareSingletonFetchQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<SingletonContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->Fetch ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareSingletonModifyQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<SingletonContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->Modify ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareShortTermInsertQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<ShortTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->Insert ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareShortTermFetchQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<ShortTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->Fetch ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareShortTermModifyQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<ShortTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->Modify ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermInsertQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->Insert ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchValueQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->FetchValue (_task.keyFields));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyValueQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->ModifyValue (_task.keyFields));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchAscendingRangeQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->FetchAscendingRange (_task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyAscendingRangeQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->ModifyAscendingRange (_task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchDescendingRangeQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->FetchDescendingRange (_task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyDescendingRangeQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));
    AddObject<PreparedQuery> (_context, _task.queryName, container->ModifyDescendingRange (_task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchShapeIntersectionQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));

    AddObject<PreparedQuery> (_context, _task.queryName,
                              container->FetchShapeIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyShapeIntersectionQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));

    AddObject<PreparedQuery> (_context, _task.queryName,
                              container->ModifyShapeIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchRayIntersectionQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));

    AddObject<PreparedQuery> (_context, _task.queryName,
                              container->FetchRayIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyRayIntersectionQuery &_task)
{
    auto &container =
        std::get<Handling::Handle<LongTermContainer>> (GetObject<ContainerReference> (_context, _task.containerName));

    AddObject<PreparedQuery> (_context, _task.queryName,
                              container->ModifyRayIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecuteTask (ExecutionContext &_context, const InsertObjects &_task)
{
    std::visit (
        [&_task] (auto &_query)
        {
            using QueryType = std::decay_t<decltype (_query)>;
            if constexpr (std::is_same_v<QueryType, ShortTermContainer::InsertQuery> ||
                          std::is_same_v<QueryType, LongTermContainer::InsertQuery>)
            {
                auto cursor = _query.Execute ();
                for (const void *source : _task.copyFrom)
                {
                    void *target = ++cursor;
                    memcpy (target, source, _query.GetContainer ()->GetTypeMapping ().GetObjectSize ());
                }
            }
            else if constexpr (std::is_same_v<QueryType, SingletonContainer::ModifyQuery>)
            {
                REQUIRE (_task.copyFrom.size () == 1u);
                auto cursor = _query.Execute ();
                memcpy (*cursor, _task.copyFrom[0u], _query.GetContainer ()->GetTypeMapping ().GetObjectSize ());
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Prepared query with name \"", _task.name, "\" must be insertion query.");
            }
        },
        GetObject<PreparedQuery> (_context, _task.name));
}

void ExecuteTask (ExecutionContext &_context, const QuerySingletonToRead &_task)
{
    auto &query = std::get<SingletonContainer::FetchQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QuerySingletonToEdit &_task)
{
    auto &query = std::get<SingletonContainer::ModifyQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QueryUnorderedSequenceToRead &_task)
{
    auto &query = std::get<ShortTermContainer::FetchQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QueryUnorderedSequenceToEdit &_task)
{
    auto &query = std::get<ShortTermContainer::ModifyQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToRead &_task)
{
    auto &query = std::get<LongTermContainer::FetchValueQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToEdit &_task)
{
    auto &query = std::get<LongTermContainer::ModifyValueQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryAscendingRangeToRead &_task)
{
    auto &query =
        std::get<LongTermContainer::FetchAscendingRangeQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryAscendingRangeToEdit &_task)
{
    auto &query =
        std::get<LongTermContainer::ModifyAscendingRangeQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryDescendingRangeToRead &_task)
{
    auto &query =
        std::get<LongTermContainer::FetchDescendingRangeQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryDescendingRangeToEdit &_task)
{
    auto &query =
        std::get<LongTermContainer::ModifyDescendingRangeQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));
    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToRead &_task)
{
    auto &query = std::get<LongTermContainer::FetchShapeIntersectionQuery> (
        GetObject<PreparedQuery> (_context, _task.sourceName));

    std::vector<uint8_t> sequence =
        Query::Test::LayoutShapeIntersectionQueryParameters (_task, CollectVolumetricQueryKeyFieldSizes (query));

    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToEdit &_task)
{
    auto &query = std::get<LongTermContainer::ModifyShapeIntersectionQuery> (
        GetObject<PreparedQuery> (_context, _task.sourceName));

    std::vector<uint8_t> sequence =
        Query::Test::LayoutShapeIntersectionQueryParameters (_task, CollectVolumetricQueryKeyFieldSizes (query));

    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToRead &_task)
{
    auto &query =
        std::get<LongTermContainer::FetchRayIntersectionQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));

    std::vector<uint8_t> sequence =
        Query::Test::LayoutRayIntersectionQueryParameters (_task, CollectVolumetricQueryKeyFieldSizes (query));

    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (&sequence[0u], _task.maxDistance));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToEdit &_task)
{
    auto &query =
        std::get<LongTermContainer::ModifyRayIntersectionQuery> (GetObject<PreparedQuery> (_context, _task.sourceName));

    std::vector<uint8_t> sequence =
        Query::Test::LayoutRayIntersectionQueryParameters (_task, CollectVolumetricQueryKeyFieldSizes (query));

    AddObject<Cursor> (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
                       query.Execute (&sequence[0u], _task.maxDistance));
}

std::ostream &operator<< (std::ostream &_output, const AcquireSingletonContainer &_task)
{
    return _output << "Acquire singleton container \"" << _task.name << "\" for mapping "
                   << *reinterpret_cast<const void *const *> (&_task.mapping) << ".";
}

std::ostream &operator<< (std::ostream &_output, const AcquireShortTermContainer &_task)
{
    return _output << "Acquire short term container \"" << _task.name << "\" for mapping "
                   << *reinterpret_cast<const void *const *> (&_task.mapping) << ".";
}

std::ostream &operator<< (std::ostream &_output, const AcquireLongTermContainer &_task)
{
    return _output << "Acquire long term container \"" << _task.name << "\" for mapping "
                   << *reinterpret_cast<const void *const *> (&_task.mapping) << ".";
}

std::ostream &operator<< (std::ostream &_output, const CheckIsSingletonContainerAllocated &_task)
{
    return _output << "Check is singleton container for mapping "
                   << *reinterpret_cast<const void *const *> (&_task.mapping)
                   << " allocated. Expected: " << (_task.expected ? "yes" : "no") << ".";
}

std::ostream &operator<< (std::ostream &_output, const CheckIsShortTermContainerAllocated &_task)
{
    return _output << "Check is short term container for mapping "
                   << *reinterpret_cast<const void *const *> (&_task.mapping)
                   << " allocated. Expected: " << (_task.expected ? "yes" : "no") << ".";
}

std::ostream &operator<< (std::ostream &_output, const CheckIsLongTermContainerAllocated &_task)
{
    return _output << "Check is long term container for mapping "
                   << *reinterpret_cast<const void *const *> (&_task.mapping)
                   << " allocated. Expected: " << (_task.expected ? "yes" : "no") << ".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareSingletonFetchQuery &_task)
{
    return _output << "Prepare singleton fetch query \"" << _task.queryName << "\" for container \""
                   << _task.containerName << "\".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareSingletonModifyQuery &_task)
{
    return _output << "Prepare singleton modify query \"" << _task.queryName << "\" for container \""
                   << _task.containerName << "\".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareShortTermInsertQuery &_task)
{
    return _output << "Prepare insert query \"" << _task.queryName << "\" for short term container \""
                   << _task.containerName << "\".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareShortTermFetchQuery &_task)
{
    return _output << "Prepare fetch query \"" << _task.queryName << "\" for short term container \""
                   << _task.containerName << "\".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareShortTermModifyQuery &_task)
{
    return _output << "Prepare modify query \"" << _task.queryName << "\" for short term container \""
                   << _task.containerName << "\".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermInsertQuery &_task)
{
    return _output << "Prepare insert query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\".";
}

std::ostream &operator<< (std::ostream &_output, const std::vector<StandardLayout::FieldId> &_fields)
{
    for (StandardLayout::FieldId id : _fields)
    {
        _output << " " << id;
    }

    return _output;
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermFetchValueQuery &_task)
{
    return _output << "Prepare fetch value query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\" on fields" << _task.keyFields << ".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermModifyValueQuery &_task)
{
    return _output << "Prepare modify value query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\" on fields" << _task.keyFields << ".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermFetchAscendingRangeQuery &_task)
{
    return _output << "Prepare fetch ascending range query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\" on field " << _task.keyField << ".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermModifyAscendingRangeQuery &_task)
{
    return _output << "Prepare modify ascending range query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\" on field " << _task.keyField << ".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermFetchDescendingRangeQuery &_task)
{
    return _output << "Prepare fetch descending range query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\" on field " << _task.keyField << ".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermModifyDescendingRangeQuery &_task)
{
    return _output << "Prepare modify descending range query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\" on field " << _task.keyField << ".";
}

std::ostream &operator<< (std::ostream &_output,
                          const std::vector<Query::Test::Sources::Volumetric::Dimension> &_dimensions)
{
    for (const auto &dimension : _dimensions)
    {
        _output << " {minField: " << dimension.minField << ", globalMin: " << dimension.globalMin
                << ", maxField: " << dimension.maxField << ", globalMax: " << dimension.maxField << "}";
    }

    return _output;
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermFetchShapeIntersectionQuery &_task)
{
    return _output << "Prepare fetch shape intersections query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\" on dimensions" << _task.dimensions << ".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermModifyShapeIntersectionQuery &_task)
{
    return _output << "Prepare modify shape intersections query \"" << _task.queryName
                   << "\" for long term container \"" << _task.containerName << "\" on dimensions" << _task.dimensions
                   << ".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermFetchRayIntersectionQuery &_task)
{
    return _output << "Prepare fetch ray intersections query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\" on dimensions" << _task.dimensions << ".";
}

std::ostream &operator<< (std::ostream &_output, const PrepareLongTermModifyRayIntersectionQuery &_task)
{
    return _output << "Prepare modify ray intersections query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\" on dimensions" << _task.dimensions << ".";
}

std::ostream &operator<< (std::ostream &_output, const InsertObjects &_task)
{
    _output << "Allocate objects using query \"" << _task.name << "\" and init from";
    for (const void *object : _task.copyFrom)
    {
        _output << " " << object;
    }

    return _output << ".";
}

namespace TestQueryApiImporters
{
template <typename ExternalTask>
Task ImportTask (const ExternalTask &_task)
{
    return _task;
}

template <>
Task ImportTask (const QuerySingletonToRead &_task)
{
    return QuerySingletonToRead {{_task.sourceName + "::Fetch", _task.cursorName}};
}

template <>
Task ImportTask (const QuerySingletonToEdit &_task)
{
    return QuerySingletonToEdit {{_task.sourceName + "::Modify", _task.cursorName}};
}

template <>
Task ImportTask (const QueryUnorderedSequenceToRead &_task)
{
    return QueryUnorderedSequenceToRead {{_task.sourceName + "::Fetch", _task.cursorName}};
}

template <>
Task ImportTask (const QueryUnorderedSequenceToEdit &_task)
{
    return QueryUnorderedSequenceToEdit {{_task.sourceName + "::Modify", _task.cursorName}};
}

template <>
Task ImportTask (const QueryValueToRead &_task)
{
    return QueryValueToRead {{{_task.sourceName + "::Fetch", _task.cursorName}, _task.value}};
}

template <>
Task ImportTask (const QueryValueToEdit &_task)
{
    return QueryValueToEdit {{{_task.sourceName + "::Modify", _task.cursorName}, _task.value}};
}

template <>
Task ImportTask (const QueryAscendingRangeToRead &_task)
{
    return QueryAscendingRangeToRead {
        {{_task.sourceName + "::FetchAscending", _task.cursorName}, _task.minValue, _task.maxValue}};
}

template <>
Task ImportTask (const QueryAscendingRangeToEdit &_task)
{
    return QueryAscendingRangeToEdit {
        {{_task.sourceName + "::ModifyAscending", _task.cursorName}, _task.minValue, _task.maxValue}};
}

template <>
Task ImportTask (const QueryDescendingRangeToRead &_task)
{
    return QueryDescendingRangeToRead {
        {{_task.sourceName + "::FetchDescending", _task.cursorName}, _task.minValue, _task.maxValue}};
}

template <>
Task ImportTask (const QueryDescendingRangeToEdit &_task)
{
    return QueryDescendingRangeToEdit {
        {{_task.sourceName + "::ModifyDescending", _task.cursorName}, _task.minValue, _task.maxValue}};
}

template <>
Task ImportTask (const QueryShapeIntersectionToRead &_task)
{
    return QueryShapeIntersectionToRead {{{_task.sourceName + "::FetchShape", _task.cursorName}, _task.min, _task.max}};
}

template <>
Task ImportTask (const QueryShapeIntersectionToEdit &_task)
{
    return QueryShapeIntersectionToEdit {
        {{_task.sourceName + "::ModifyShape", _task.cursorName}, _task.min, _task.max}};
}

template <>
Task ImportTask (const QueryRayIntersectionToRead &_task)
{
    return QueryRayIntersectionToRead {
        {{_task.sourceName + "::FetchRay", _task.cursorName}, _task.origin, _task.direction, _task.maxDistance}};
}

template <>
Task ImportTask (const QueryRayIntersectionToEdit &_task)
{
    return QueryRayIntersectionToEdit {
        {{_task.sourceName + "::ModifyRay", _task.cursorName}, _task.origin, _task.direction, _task.maxDistance}};
}

static Task ContainerAcquisitionTaskFromSource (const Query::Test::Source &_source,
                                                const StandardLayout::Mapping &_typeMapping,
                                                const std::string &_containerName)
{
    if (std::holds_alternative<Query::Test::Sources::Singleton> (_source))
    {
        return AcquireSingletonContainer {{_typeMapping, _containerName}};
    }
    else if (std::holds_alternative<Query::Test::Sources::UnorderedSequence> (_source))
    {
        return AcquireShortTermContainer {{_typeMapping, _containerName}};
    }
    else
    {
        const bool isParametricSource = std::holds_alternative<Query::Test::Sources::Value> (_source) ||
                                        std::holds_alternative<Query::Test::Sources::Range> (_source) ||
                                        std::holds_alternative<Query::Test::Sources::Volumetric> (_source);

        REQUIRE (isParametricSource);
        return AcquireLongTermContainer {{_typeMapping, _containerName}};
    }
}

static std::vector<Task> ImportStorage (const Query::Test::Storage &_storage,
                                        const std::string &_containerName,
                                        const bool _prepareQueries)
{
    std::vector<Task> tasks;
    Task containerAcquisition =
        ContainerAcquisitionTaskFromSource (_storage.sources.front (), _storage.dataType, _containerName);
    tasks.emplace_back (containerAcquisition);

    if (std::holds_alternative<AcquireSingletonContainer> (containerAcquisition))
    {
        REQUIRE (_storage.objectsToInsert.size () == 1u);
        tasks.emplace_back (PrepareSingletonModifyQuery {{_containerName, _containerName + "::Init"}});
    }
    else if (std::holds_alternative<AcquireShortTermContainer> (containerAcquisition))
    {
        tasks.emplace_back (PrepareShortTermInsertQuery {{_containerName, _containerName + "::Init"}});
    }
    else
    {
        REQUIRE (std::holds_alternative<AcquireLongTermContainer> (containerAcquisition));
        tasks.emplace_back (PrepareLongTermInsertQuery {{_containerName, _containerName + "::Init"}});
    }

    tasks.emplace_back (InsertObjects {_containerName + "::Init", _storage.objectsToInsert});
    tasks.emplace_back (Delete<PreparedQueryTag> {{_containerName + "::Init"}});

    for (const auto &source : _storage.sources)
    {
        // Storage is always represented by container and containers
        // only support specific subsets of sources, not all of them.
        REQUIRE (
            containerAcquisition.index () ==
            ContainerAcquisitionTaskFromSource (_storage.sources.front (), _storage.dataType, _containerName).index ());

        if (!_prepareQueries)
        {
            continue;
        }

        std::visit (
            [&tasks, &_containerName] (const auto &_source)
            {
                using SourceType = std::decay_t<decltype (_source)>;

                // For simplicity, we prepare all possible type of queries for given sources.

                if constexpr (std::is_same_v<SourceType, Query::Test::Sources::Singleton>)
                {
                    tasks.emplace_back (PrepareSingletonFetchQuery {{_containerName, _source.name + "::Fetch"}});

                    tasks.emplace_back (PrepareSingletonModifyQuery {{_containerName, _source.name + "::Modify"}});
                }
                else if constexpr (std::is_same_v<SourceType, Query::Test::Sources::UnorderedSequence>)
                {
                    tasks.emplace_back (PrepareShortTermFetchQuery {{_containerName, _source.name + "::Fetch"}});

                    tasks.emplace_back (PrepareShortTermModifyQuery {{_containerName, _source.name + "::Modify"}});
                }
                else if constexpr (std::is_same_v<SourceType, Query::Test::Sources::Value>)
                {
                    tasks.emplace_back (PrepareLongTermFetchValueQuery {{_containerName, _source.name + "::Fetch"},
                                                                        _source.queriedFields});

                    tasks.emplace_back (PrepareLongTermModifyValueQuery {{_containerName, _source.name + "::Modify"},
                                                                         _source.queriedFields});
                }
                else if constexpr (std::is_same_v<SourceType, Query::Test::Sources::Range>)
                {
                    tasks.emplace_back (PrepareLongTermFetchAscendingRangeQuery {
                        {_containerName, _source.name + "::FetchAscending"}, _source.queriedField});

                    tasks.emplace_back (PrepareLongTermModifyAscendingRangeQuery {
                        {_containerName, _source.name + "::ModifyAscending"}, _source.queriedField});

                    tasks.emplace_back (PrepareLongTermFetchDescendingRangeQuery {
                        {_containerName, _source.name + "::FetchDescending"}, _source.queriedField});

                    tasks.emplace_back (PrepareLongTermModifyDescendingRangeQuery {
                        {_containerName, _source.name + "::ModifyDescending"}, _source.queriedField});
                }
                else if constexpr (std::is_same_v<SourceType, Query::Test::Sources::Volumetric>)
                {
                    tasks.emplace_back (PrepareLongTermFetchShapeIntersectionQuery {
                        {_containerName, _source.name + "::FetchShape"}, _source.dimensions});

                    tasks.emplace_back (PrepareLongTermModifyShapeIntersectionQuery {
                        {_containerName, _source.name + "::ModifyShape"}, _source.dimensions});

                    tasks.emplace_back (PrepareLongTermFetchRayIntersectionQuery {
                        {_containerName, _source.name + "::FetchRay"}, _source.dimensions});

                    tasks.emplace_back (PrepareLongTermModifyRayIntersectionQuery {
                        {_containerName, _source.name + "::ModifyRay"}, _source.dimensions});
                }
            },
            source);
    }

    return tasks;
}
} // namespace TestQueryApiImporters

void TestQueryApiDriver (const Query::Test::Scenario &_scenario)
{
    using namespace TestQueryApiImporters;
    std::vector<Task> tasks;

    for (std::size_t index = 0u; index < _scenario.storages.size (); ++index)
    {
        const Query::Test::Storage &storage = _scenario.storages[index];
        if (!storage.sources.empty ())
        {
            tasks += ImportStorage (storage, std::to_string (index), true);
        }
    }

    for (const auto &task : _scenario.tasks)
    {
        std::visit (
            [&tasks] (const auto &_task)
            {
                tasks.emplace_back (ImportTask (_task));
            },
            task);
    }

    Scenario {tasks};
}

namespace TestReferenceApiDrivers
{
static Task ContainerAcquisitionTaskToAllocationCheck (const Task &_acquisition, const bool _allocationExpected)
{
    if (const auto *singleton = std::get_if<AcquireSingletonContainer> (&_acquisition))
    {
        return CheckIsSingletonContainerAllocated {{singleton->mapping, _allocationExpected}};
    }
    else if (const auto *shortTerm = std::get_if<AcquireShortTermContainer> (&_acquisition))
    {
        return CheckIsShortTermContainerAllocated {{shortTerm->mapping, _allocationExpected}};
    }
    else
    {
        const auto *longTerm = std::get_if<AcquireLongTermContainer> (&_acquisition);
        REQUIRE (longTerm);
        return CheckIsLongTermContainerAllocated {{longTerm->mapping, _allocationExpected}};
    }
}

void ForContainerReference (const Reference::Test::Scenario &_scenario,
                            const Query::Test::Storage &_containerDescriptor)
{
    std::vector<Task> tasks;
    bool containerCreated = false;
    bool anyReferenceModificationExecuted = false;

    for (const Reference::Test::Task &packedTask : _scenario)
    {
        std::visit (
            [&tasks, &anyReferenceModificationExecuted, &containerCreated, &_containerDescriptor] (const auto &_task)
            {
                using TaskType = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Create>)
                {
                    if (containerCreated)
                    {
                        tasks.emplace_back (TestQueryApiImporters::ContainerAcquisitionTaskFromSource (
                            _containerDescriptor.sources[0u], _containerDescriptor.dataType, _task.name));
                    }
                    else
                    {
                        REQUIRE_WITH_MESSAGE (
                            !anyReferenceModificationExecuted,
                            "For simplicity container creation after reference modification is not supported, "
                            "because it would introduce branching into test.");

                        tasks += TestQueryApiImporters::ImportStorage (_containerDescriptor, _task.name, false);
                        containerCreated = true;
                    }
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::CheckStatus>)
                {
                    tasks.emplace_back (ContainerAcquisitionTaskToAllocationCheck (
                        TestQueryApiImporters::ContainerAcquisitionTaskFromSource (_containerDescriptor.sources[0u],
                                                                                   _containerDescriptor.dataType, {}),
                        _task.hasAnyReferences));
                }
                else
                {
                    anyReferenceModificationExecuted = true;
                    if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Move>)
                    {
                        tasks.emplace_back (Move<ContainerReferenceTag> {_task.sourceName, _task.targetName});
                    }
                    else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Copy>)
                    {
                        tasks.emplace_back (Copy<ContainerReferenceTag> {_task.sourceName, _task.targetName});
                    }
                    else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Delete>)
                    {
                        tasks.emplace_back (Delete<ContainerReferenceTag> {_task.name});
                    }
                    else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::MoveAssign>)
                    {
                        tasks.emplace_back (MoveAssign<ContainerReferenceTag> {_task.sourceName, _task.targetName});
                    }
                    else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::CopyAssign>)
                    {
                        tasks.emplace_back (CopyAssign<ContainerReferenceTag> {_task.sourceName, _task.targetName});
                    }
                }
            },
            packedTask);
    }

    Scenario {tasks};
}

static Task RenamePreparedQuery (Task _queryPreparation, std::string _newContainerName, std::string _newQueryName)
{
    std::visit (
        [&_newContainerName, &_newQueryName] (auto &_task)
        {
            if constexpr (std::is_base_of_v<QueryPreparationBase, std::decay_t<decltype (_task)>>)
            {
                _task.containerName = _newContainerName;
                _task.queryName = _newQueryName;
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Query preparation task should inherit QueryPreparationBase.");
            }
        },
        _queryPreparation);

    return _queryPreparation;
}

void ForPreparedQuery (const Reference::Test::Scenario &_scenario,
                       const Query::Test::Storage &_containerDescriptor,
                       const Task &_queryPreparation)
{
    // TODO: A lot of duplication with ::ForContainerReference, but it's tricky to get rid of it nicely.
    std::vector<Task> tasks;
    bool containerCreated = false;
    bool anyReferenceModificationExecuted = false;

    for (const Reference::Test::Task &packedTask : _scenario)
    {
        std::visit (
            [&tasks, &anyReferenceModificationExecuted, &containerCreated, &_containerDescriptor,
             &_queryPreparation] (const auto &_task)
            {
                using TaskType = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Create>)
                {
                    if (containerCreated)
                    {
                        tasks.emplace_back (TestQueryApiImporters::ContainerAcquisitionTaskFromSource (
                            _containerDescriptor.sources[0u], _containerDescriptor.dataType, "TemporaryContainer"));
                    }
                    else
                    {
                        REQUIRE_WITH_MESSAGE (
                            !anyReferenceModificationExecuted,
                            "For simplicity container creation after reference modification is not supported, "
                            "because it would introduce branching into test.");

                        tasks +=
                            TestQueryApiImporters::ImportStorage (_containerDescriptor, "TemporaryContainer", false);
                        containerCreated = true;
                    }

                    tasks.emplace_back (RenamePreparedQuery (_queryPreparation, "TemporaryContainer", _task.name));
                    tasks.emplace_back (Delete<ContainerReferenceTag> {{"TemporaryContainer"}});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::CheckStatus>)
                {
                    tasks.emplace_back (ContainerAcquisitionTaskToAllocationCheck (
                        TestQueryApiImporters::ContainerAcquisitionTaskFromSource (_containerDescriptor.sources[0u],
                                                                                   _containerDescriptor.dataType, {}),
                        _task.hasAnyReferences));
                }
                else
                {
                    anyReferenceModificationExecuted = true;
                    if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Move>)
                    {
                        tasks.emplace_back (Move<PreparedQueryTag> {_task.sourceName, _task.targetName});
                    }
                    else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Copy>)
                    {
                        tasks.emplace_back (Copy<PreparedQueryTag> {_task.sourceName, _task.targetName});
                    }
                    else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Delete>)
                    {
                        tasks.emplace_back (Delete<PreparedQueryTag> {_task.name});
                    }
                }
            },
            packedTask);
    }

    Scenario {tasks};
}

void ForCursor (const Reference::Test::Scenario &_scenario,
                const Query::Test::Storage &_containerDescriptor,
                const Query::Test::Task &_query,
                const void *_cursorExpectedObject)
{
    std::vector<Task> tasks = TestQueryApiImporters::ImportStorage (_containerDescriptor, "Container", true);
    for (const Reference::Test::Task &packedTask : _scenario)
    {
        std::visit (
            [&tasks, &_query, _cursorExpectedObject] (const auto &_task)
            {
                using TaskType = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Create>)
                {
                    std::visit (
                        [&tasks] (const auto &_query)
                        {
                            tasks.emplace_back (TestQueryApiImporters::ImportTask (_query));
                        },
                        Query::Test::ChangeQuerySourceAndCursor (_query, std::nullopt, _task.name));

                    tasks.emplace_back (CursorCheck {_task.name, _cursorExpectedObject});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::CheckStatus>)
                {
                    // Cursor creation/destruction doesn't change status of prepared queries and containers,
                    // therefore we just skip this task.
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Move>)
                {
                    tasks.emplace_back (Move<CursorTag> {_task.sourceName, _task.targetName});
                    tasks.emplace_back (CursorCheck {_task.targetName, _cursorExpectedObject});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Copy>)
                {
                    tasks.emplace_back (Copy<CursorTag> {_task.sourceName, _task.targetName});
                    tasks.emplace_back (CursorCheck {_task.targetName, _cursorExpectedObject});
                }
                else if constexpr (std::is_same_v<TaskType, Reference::Test::Tasks::Delete>)
                {
                    tasks.emplace_back (Delete<CursorTag> {_task.name});
                }
            },
            packedTask);
    }

    Scenario {tasks};
}
} // namespace TestReferenceApiDrivers

Scenario::Scenario (std::vector<Task> _tasks) : tasks (std::move (_tasks))
{
    ExecutionContext context;
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

std::vector<Task> &operator+= (std::vector<Task> &first, const std::vector<Task> &second) noexcept
{
    for (const Task &task : second)
    {
        first.emplace_back (task);
    }

    return first;
}

std::vector<Task> operator+ (std::vector<Task> first, const Task &_task) noexcept
{
    first.emplace_back (_task);
    return first;
}

} // namespace Emergence::Galleon::Test
