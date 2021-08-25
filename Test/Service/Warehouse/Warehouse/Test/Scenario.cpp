#include <optional>
#include <sstream>
#include <unordered_map>

#include <Query/Test/CursorStorage.hpp>

#include <Testing/Testing.hpp>

#include <Warehouse/Registry.hpp>
#include <Warehouse/Test/Scenario.hpp>

namespace Emergence::Warehouse::Test
{
using PreparedQuery = std::variant <
    FetchSingletonQuery,
    ModifySingletonQuery,
    InsertShortTermQuery,
    FetchSequenceQuery,
    ModifySequenceQuery,
    InsertLongTermQuery,
    FetchValueQuery,
    ModifyValueQuery,
    FetchAscendingRangeQuery,
    ModifyAscendingRangeQuery,
    FetchDescendingRangeQuery,
    ModifyDescendingRangeQuery,
    FetchShapeIntersectionQuery,
    ModifyShapeIntersectionQuery,
    FetchRayIntersectionQuery,
    ModifyRayIntersectionQuery>;

using Cursor = std::variant <
    FetchSingletonQuery::Cursor,
    ModifySingletonQuery::Cursor,
    FetchSequenceQuery::Cursor,
    ModifySequenceQuery::Cursor,
    FetchValueQuery::Cursor,
    ModifyValueQuery::Cursor,
    FetchAscendingRangeQuery::Cursor,
    ModifyAscendingRangeQuery::Cursor,
    FetchDescendingRangeQuery::Cursor,
    ModifyDescendingRangeQuery::Cursor,
    FetchShapeIntersectionQuery::Cursor,
    ModifyShapeIntersectionQuery::Cursor,
    FetchRayIntersectionQuery::Cursor,
    ModifyRayIntersectionQuery::Cursor>;
} // namespace Emergence::Warehouse::Test

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (
    Emergence::Warehouse::Test::CursorTag,
    Emergence::Query::Test::CursorData <Emergence::Warehouse::Test::Cursor>,
    "cursor")

EMERGENCE_CONTEXT_BIND_OBJECT_TAG (
    Emergence::Warehouse::Test::PreparedQueryTag, Emergence::Warehouse::Test::PreparedQuery, "prepared query")

namespace Emergence::Warehouse::Test
{
struct ExecutionContext final :
    public Context::Extension::ObjectStorage <PreparedQuery>,
    public Query::Test::CursorStorage <Cursor>
{
    ExecutionContext () = default;

    ~ExecutionContext ();

    Registry registry;
};

ExecutionContext::~ExecutionContext ()
{
    Query::Test::CursorStorage <Cursor>::objects.clear ();
    Context::Extension::ObjectStorage <PreparedQuery>::objects.clear ();
}

std::vector <Warehouse::Dimension> ConvertDimensions (
    const StandardLayout::Mapping &_typeMapping,
    const std::vector <Query::Test::Sources::Volumetric::Dimension> &_dimensions)
{
    std::vector <Warehouse::Dimension> convertedDimensions;
    convertedDimensions.reserve (_dimensions.size ());

    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _dimensions)
    {
        convertedDimensions.emplace_back (
            Warehouse::Dimension
                {
                    &dimension.globalMin,
                    _typeMapping.GetField (dimension.minField),
                    &dimension.globalMax,
                    _typeMapping.GetField (dimension.maxField),
                });
    }

    return convertedDimensions;
}

template <typename QueryType>
std::vector <std::size_t> CollectVolumetricQueryKeyFieldSizes (const QueryType &_query)
{
    std::vector <std::size_t> result;
    for (auto iterator = _query.DimensionBegin (); iterator != _query.DimensionEnd (); ++iterator)
    {
        auto dimension = *iterator;
        REQUIRE (dimension.minBorderField.GetSize () == dimension.maxBorderField.GetSize ());
        result.emplace_back (dimension.minBorderField.GetSize ());
    }

    return result;
}

void ExecuteTask (ExecutionContext &_context, const PrepareFetchSingletonQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.FetchSingleton (_task.typeMapping));
}

void ExecuteTask (ExecutionContext &_context, const PrepareModifySingletonQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.ModifySingleton (_task.typeMapping));
}

void ExecuteTask (ExecutionContext &_context, const PrepareInsertShortTermQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.InsertShortTerm (_task.typeMapping));
}

void ExecuteTask (ExecutionContext &_context, const PrepareFetchSequenceQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.FetchSequence (_task.typeMapping));
}

void ExecuteTask (ExecutionContext &_context, const PrepareModifySequenceQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.ModifySequence (_task.typeMapping));
}

void ExecuteTask (ExecutionContext &_context, const PrepareInsertLongTermQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.InsertLongTerm (_task.typeMapping));
}

void ExecuteTask (ExecutionContext &_context, const PrepareFetchValueQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.FetchValue (_task.typeMapping, _task.keyFields));
}

void ExecuteTask (ExecutionContext &_context, const PrepareModifyValueQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.ModifyValue (_task.typeMapping, _task.keyFields));
}

void ExecuteTask (ExecutionContext &_context, const PrepareFetchAscendingRangeQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.FetchAscendingRange (_task.typeMapping, _task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareModifyAscendingRangeQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.ModifyAscendingRange (_task.typeMapping, _task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareFetchDescendingRangeQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.FetchDescendingRange (_task.typeMapping, _task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareModifyDescendingRangeQuery &_task)
{
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.ModifyDescendingRange (_task.typeMapping, _task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareFetchShapeIntersectionQuery &_task)
{
    std::vector <Warehouse::Dimension> dimensions = ConvertDimensions (_task.typeMapping, _task.dimensions);
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.FetchShapeIntersection (_task.typeMapping, dimensions));
}

void ExecuteTask (ExecutionContext &_context, const PrepareModifyShapeIntersectionQuery &_task)
{
    std::vector <Warehouse::Dimension> dimensions = ConvertDimensions (_task.typeMapping, _task.dimensions);
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.ModifyShapeIntersection (_task.typeMapping, dimensions));
}

void ExecuteTask (ExecutionContext &_context, const PrepareFetchRayIntersectionQuery &_task)
{
    std::vector <Warehouse::Dimension> dimensions = ConvertDimensions (_task.typeMapping, _task.dimensions);
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.FetchRayIntersection (_task.typeMapping, dimensions));
}

void ExecuteTask (ExecutionContext &_context, const PrepareModifyRayIntersectionQuery &_task)
{
    std::vector <Warehouse::Dimension> dimensions = ConvertDimensions (_task.typeMapping, _task.dimensions);
    AddObject <PreparedQuery> (
        _context, _task.queryName, _context.registry.ModifyRayIntersection (_task.typeMapping, dimensions));
}

void ExecuteTask (ExecutionContext &_context, const InsertObjects &_task)
{
    std::visit (
        [&_task] (auto &_query)
        {
            using QueryType = std::decay_t <decltype (_query)>;
            if constexpr (
                std::is_same_v <QueryType, InsertShortTermQuery> || std::is_same_v <QueryType, InsertLongTermQuery>)
            {
                auto cursor = _query.Execute ();
                for (const void *source : _task.copyFrom)
                {
                    void *target = ++cursor;
                    memcpy (target, source, _query.GetTypeMapping ().GetObjectSize ());
                }
            }
            else if constexpr (std::is_same_v <QueryType, ModifySingletonQuery>)
            {
                REQUIRE (_task.copyFrom.size () == 1u);
                auto cursor = _query.Execute ();
                memcpy (*cursor, _task.copyFrom[0u],
                        _query.GetTypeMapping ().GetObjectSize ());
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Prepared query with name \"", _task.name, "\" must be insertion query.");
            }
        },
        GetObject <PreparedQuery> (_context, _task.name));
}

void ExecuteTask (ExecutionContext &_context, const QuerySingletonToRead &_task)
{
    auto &query = std::get <FetchSingletonQuery> (GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QuerySingletonToEdit &_task)
{
    auto &query = std::get <ModifySingletonQuery> (GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QueryUnorderedSequenceToRead &_task)
{
    auto &query = std::get <FetchSequenceQuery> (GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QueryUnorderedSequenceToEdit &_task)
{
    auto &query = std::get <ModifySequenceQuery> (GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToRead &_task)
{
    auto &query = std::get <FetchValueQuery> (GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (), query.Execute (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToEdit &_task)
{
    auto &query = std::get <ModifyValueQuery> (GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (), query.Execute (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryAscendingRangeToRead &_task)
{
    auto &query = std::get <FetchAscendingRangeQuery> (GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (),
                        query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryAscendingRangeToEdit &_task)
{
    auto &query = std::get <ModifyAscendingRangeQuery> (
        GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (),
                        query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryDescendingRangeToRead &_task)
{
    auto &query = std::get <FetchDescendingRangeQuery> (
        GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (),
                        query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryDescendingRangeToEdit &_task)
{
    auto &query = std::get <ModifyDescendingRangeQuery> (
        GetObject <PreparedQuery> (_context, _task.sourceName));
    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (),
                        query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToRead &_task)
{
    auto &query = std::get <FetchShapeIntersectionQuery> (
        GetObject <PreparedQuery> (_context, _task.sourceName));

    std::vector <uint8_t> sequence = Query::Test::LayoutShapeIntersectionQueryParameters (
        _task, CollectVolumetricQueryKeyFieldSizes (query));

    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (),
                        query.Execute (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToEdit &_task)
{
    auto &query = std::get <ModifyShapeIntersectionQuery> (
        GetObject <PreparedQuery> (_context, _task.sourceName));

    std::vector <uint8_t> sequence = Query::Test::LayoutShapeIntersectionQueryParameters (
        _task, CollectVolumetricQueryKeyFieldSizes (query));

    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (),
                        query.Execute (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToRead &_task)
{
    auto &query = std::get <FetchRayIntersectionQuery> (
        GetObject <PreparedQuery> (_context, _task.sourceName));

    std::vector <uint8_t> sequence = Query::Test::LayoutRayIntersectionQueryParameters (
        _task, CollectVolumetricQueryKeyFieldSizes (query));

    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (),
                        query.Execute (&sequence[0u], _task.maxDistance));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToEdit &_task)
{
    auto &query = std::get <ModifyRayIntersectionQuery> (
        GetObject <PreparedQuery> (_context, _task.sourceName));

    std::vector <uint8_t> sequence = Query::Test::LayoutRayIntersectionQueryParameters (
        _task, CollectVolumetricQueryKeyFieldSizes (query));

    AddObject <Cursor> (_context, _task.cursorName, query.GetTypeMapping (),
                        query.Execute (&sequence[0u], _task.maxDistance));
}

std::ostream &operator << (std::ostream &_output, const PrepareFetchSingletonQuery &_task)
{
    return _output << "Prepare fetch singleton query \"" << _task.queryName << "\" for type mapping \""
                   << *reinterpret_cast <const void *const *> (&_task.typeMapping) << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareModifySingletonQuery &_task)
{
    return _output << "Prepare modify singleton query \"" << _task.queryName << "\" for type mapping \""
                   << *reinterpret_cast <const void *const *> (&_task.typeMapping) << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareInsertShortTermQuery &_task)
{
    return _output << "Prepare insert short term query \"" << _task.queryName << "\" for type mapping \""
                   << *reinterpret_cast <const void *const *> (&_task.typeMapping) << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareFetchSequenceQuery &_task)
{
    return _output << "Prepare fetch sequence query \"" << _task.queryName << "\" for type mapping \""
                   << *reinterpret_cast <const void *const *> (&_task.typeMapping) << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareModifySequenceQuery &_task)
{
    return _output << "Prepare modify sequence query \"" << _task.queryName << "\" for type mapping \""
                   << *reinterpret_cast <const void *const *> (&_task.typeMapping) << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareInsertLongTermQuery &_task)
{
    return _output << "Prepare insert long term query \"" << _task.queryName << "\" for type mapping \""
                   << *reinterpret_cast <const void *const *> (&_task.typeMapping) << "\".";
}

std::ostream &operator << (std::ostream &_output, const std::vector <StandardLayout::FieldId> &_fields)
{
    for (StandardLayout::FieldId id : _fields)
    {
        _output << " " << id;
    }

    return _output;
}

std::ostream &operator << (std::ostream &_output, const PrepareFetchValueQuery &_task)
{
    return _output << "Prepare fetch value query \"" << _task.queryName << "\" for type mapping \"" <<
                   *reinterpret_cast <const void *const *> (&_task.typeMapping) << "\" on fields" <<
                   _task.keyFields << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareModifyValueQuery &_task)
{
    return _output << "Prepare modify value query \"" << _task.queryName << "\" for type mapping \"" <<
                   *reinterpret_cast <const void *const *> (&_task.typeMapping) <<
                   "\" on fields" << _task.keyFields << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareFetchAscendingRangeQuery &_task)
{
    return _output << "Prepare fetch ascending range query \"" << _task.queryName <<
                   "\" for type mapping \"" << *reinterpret_cast <const void *const *> (&_task.typeMapping) <<
                   "\" on field " << _task.keyField << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareModifyAscendingRangeQuery &_task)
{
    return _output << "Prepare modify ascending range query \"" << _task.queryName <<
                   "\" for type mapping \"" << *reinterpret_cast <const void *const *> (&_task.typeMapping) <<
                   "\" on field " << _task.keyField << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareFetchDescendingRangeQuery &_task)
{
    return _output << "Prepare fetch descending range query \"" << _task.queryName <<
                   "\" for type mapping \"" << *reinterpret_cast <const void *const *> (&_task.typeMapping) <<
                   "\" on field " << _task.keyField << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareModifyDescendingRangeQuery &_task)
{
    return _output << "Prepare modify descending range query \"" << _task.queryName <<
                   "\" for type mapping \"" << *reinterpret_cast <const void *const *> (&_task.typeMapping) <<
                   "\" on field " << _task.keyField << ".";
}

std::ostream &operator << (
    std::ostream &_output, const std::vector <Query::Test::Sources::Volumetric::Dimension> &_dimensions)
{
    for (const auto &dimension : _dimensions)
    {
        _output << " {minField: " << dimension.minField << ", globalMin: " << dimension.globalMin <<
                ", maxField: " << dimension.maxField << ", globalMax: " << dimension.maxField << "}";
    }

    return _output;
}

std::ostream &operator << (std::ostream &_output, const PrepareFetchShapeIntersectionQuery &_task)
{
    return _output << "Prepare fetch shape intersections query \"" << _task.queryName <<
                   "\" for type mapping \"" << *reinterpret_cast <const void *const *> (&_task.typeMapping) <<
                   "\" on dimensions" << _task.dimensions << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareModifyShapeIntersectionQuery &_task)
{
    return _output << "Prepare modify shape intersections query \"" << _task.queryName <<
                   "\" for type mapping \"" << *reinterpret_cast <const void *const *> (&_task.typeMapping) <<
                   "\" on dimensions" << _task.dimensions << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareFetchRayIntersectionQuery &_task)
{
    return _output << "Prepare fetch ray intersections query \"" << _task.queryName <<
                   "\" for type mapping \"" << *reinterpret_cast <const void *const *> (&_task.typeMapping) <<
                   "\" on dimensions" << _task.dimensions << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareModifyRayIntersectionQuery &_task)
{
    return _output << "Prepare modify ray intersections query \"" << _task.queryName <<
                   "\" for type mapping \"" << *reinterpret_cast <const void *const *> (&_task.typeMapping) <<
                   "\" on dimensions" << _task.dimensions << ".";
}

std::ostream &operator << (std::ostream &_output, const InsertObjects &_task)
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
    return QueryAscendingRangeToRead {{{_task.sourceName + "::FetchAscending", _task.cursorName},
                                       _task.minValue, _task.maxValue}};
}

template <>
Task ImportTask (const QueryAscendingRangeToEdit &_task)
{
    return QueryAscendingRangeToEdit {{{_task.sourceName + "::ModifyAscending", _task.cursorName},
                                       _task.minValue, _task.maxValue}};
}

template <>
Task ImportTask (const QueryDescendingRangeToRead &_task)
{
    return QueryDescendingRangeToRead {{{_task.sourceName + "::FetchDescending", _task.cursorName},
                                        _task.minValue, _task.maxValue}};
}

template <>
Task ImportTask (const QueryDescendingRangeToEdit &_task)
{
    return QueryDescendingRangeToEdit {{{_task.sourceName + "::ModifyDescending", _task.cursorName},
                                        _task.minValue, _task.maxValue}};
}

template <>
Task ImportTask (const QueryShapeIntersectionToRead &_task)
{
    return QueryShapeIntersectionToRead {{{_task.sourceName + "::FetchShape", _task.cursorName},
                                          _task.min, _task.max}};
}

template <>
Task ImportTask (const QueryShapeIntersectionToEdit &_task)
{
    return QueryShapeIntersectionToEdit {{{_task.sourceName + "::ModifyShape", _task.cursorName},
                                          _task.min, _task.max}};
}

template <>
Task ImportTask (const QueryRayIntersectionToRead &_task)
{
    return QueryRayIntersectionToRead {{{_task.sourceName + "::FetchRay", _task.cursorName},
                                        _task.origin, _task.direction, _task.maxDistance}};
}

template <>
Task ImportTask (const QueryRayIntersectionToEdit &_task)
{
    return QueryRayIntersectionToEdit {{{_task.sourceName + "::ModifyRay", _task.cursorName},
                                        _task.origin, _task.direction, _task.maxDistance}};
}

static Task InsertQueryPreparationTaskFromSource (
    const Query::Test::Source &_source, const StandardLayout::Mapping &_typeMapping, const std::string &_queryName)
{
    if (std::holds_alternative <Query::Test::Sources::Singleton> (_source))
    {
        return PrepareModifySingletonQuery {{_typeMapping, _queryName}};
    }
    else if (std::holds_alternative <Query::Test::Sources::UnorderedSequence> (_source))
    {
        return PrepareInsertShortTermQuery {{_typeMapping, _queryName}};
    }
    else
    {
        const bool isParametricSource =
            std::holds_alternative <Query::Test::Sources::Value> (_source) ||
            std::holds_alternative <Query::Test::Sources::Range> (_source) ||
            std::holds_alternative <Query::Test::Sources::Volumetric> (_source);

        REQUIRE (isParametricSource);
        return PrepareInsertLongTermQuery {{_typeMapping, _queryName}};
    }
}

static std::vector <Task> ImportStorage (
    const Query::Test::Storage &_storage, const std::optional <Task> &_prepareOnlyCustomQuery = std::nullopt)
{
    std::vector <Task> tasks;
    Task inserterPreparation = InsertQueryPreparationTaskFromSource (
        _storage.sources.front (), _storage.dataType, "TemporaryInserter");

    if (_prepareOnlyCustomQuery)
    {
        tasks.emplace_back (_prepareOnlyCustomQuery.value ());
    }
    else
    {
        for (const auto &source : _storage.sources)
        {
            std::visit (
                [&tasks, &_storage] (const auto &_source)
                {
                    using SourceType = std::decay_t <decltype (_source)>;

                    // For simplicity, we prepare all possible type of queries for given sources.

                    if constexpr (std::is_same_v <SourceType, Query::Test::Sources::Singleton>)
                    {
                        tasks.emplace_back (
                            PrepareFetchSingletonQuery {{_storage.dataType, _source.name + "::Fetch"}});

                        tasks.emplace_back (
                            PrepareModifySingletonQuery {{_storage.dataType, _source.name + "::Modify"}});
                    }
                    else if constexpr (std::is_same_v <SourceType, Query::Test::Sources::UnorderedSequence>)
                    {
                        tasks.emplace_back (
                            PrepareFetchSequenceQuery {{_storage.dataType, _source.name + "::Fetch"}});

                        tasks.emplace_back (
                            PrepareModifySequenceQuery {{_storage.dataType, _source.name + "::Modify"}});
                    }
                    else if constexpr (std::is_same_v <SourceType, Query::Test::Sources::Value>)
                    {
                        tasks.emplace_back (
                            PrepareFetchValueQuery
                                {{_storage.dataType, _source.name + "::Fetch"}, _source.queriedFields});

                        tasks.emplace_back (
                            PrepareModifyValueQuery
                                {{_storage.dataType, _source.name + "::Modify"}, _source.queriedFields});
                    }
                    else if constexpr (std::is_same_v <SourceType, Query::Test::Sources::Range>)
                    {
                        tasks.emplace_back (
                            PrepareFetchAscendingRangeQuery
                                {{_storage.dataType, _source.name + "::FetchAscending"}, _source.queriedField});

                        tasks.emplace_back (
                            PrepareModifyAscendingRangeQuery
                                {{_storage.dataType, _source.name + "::ModifyAscending"}, _source.queriedField});

                        tasks.emplace_back (
                            PrepareFetchDescendingRangeQuery
                                {{_storage.dataType, _source.name + "::FetchDescending"}, _source.queriedField});

                        tasks.emplace_back (
                            PrepareModifyDescendingRangeQuery
                                {{_storage.dataType, _source.name + "::ModifyDescending"}, _source.queriedField});
                    }
                    else if constexpr (std::is_same_v <SourceType, Query::Test::Sources::Volumetric>)
                    {
                        tasks.emplace_back (
                            PrepareFetchShapeIntersectionQuery
                                {{_storage.dataType, _source.name + "::FetchShape"}, _source.dimensions});

                        tasks.emplace_back (
                            PrepareModifyShapeIntersectionQuery
                                {{_storage.dataType, _source.name + "::ModifyShape"}, _source.dimensions});

                        tasks.emplace_back (
                            PrepareFetchRayIntersectionQuery
                                {{_storage.dataType, _source.name + "::FetchRay"}, _source.dimensions});

                        tasks.emplace_back (
                            PrepareModifyRayIntersectionQuery
                                {{_storage.dataType, _source.name + "::ModifyRay"}, _source.dimensions});
                    }
                },
                source);
        }
    }

    tasks.emplace_back (inserterPreparation);
    tasks.emplace_back (InsertObjects {"TemporaryInserter", _storage.objectsToInsert});
    tasks.emplace_back (Delete <PreparedQueryTag> {{"TemporaryInserter"}});
    return tasks;
}
} // namespace TestQueryApiImporters

void TestQueryApiDriver (const Query::Test::Scenario &_scenario)
{
    using namespace TestQueryApiImporters;
    std::vector <Task> tasks;

    for (std::size_t index = 0u; index < _scenario.storages.size (); ++index)
    {
        const Query::Test::Storage &storage = _scenario.storages[index];
        if (!storage.sources.empty ())
        {
            tasks += ImportStorage (storage);
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
//static Task ContainerAcquisitionTaskToAllocationCheck (const Task &_acquisition, const bool _allocationExpected)
//{
//    if (const auto *singleton = std::get_if <AcquireSingletonContainer> (&_acquisition))
//    {
//        return CheckIsSingletonContainerAllocated {{singleton->mapping, _allocationExpected}};
//    }
//    else if (const auto *shortTerm = std::get_if <AcquireShortTermContainer> (&_acquisition))
//    {
//        return CheckIsShortTermContainerAllocated {{shortTerm->mapping, _allocationExpected}};
//    }
//    else
//    {
//        const auto *longTerm = std::get_if <AcquireLongTermContainer> (&_acquisition);
//        REQUIRE (longTerm);
//        return CheckIsLongTermContainerAllocated {{longTerm->mapping, _allocationExpected}};
//    }
//}
//
//static Task RenamePreparedQuery (Task _queryPreparation, std::string _newContainerName, std::string _newQueryName)
//{
//    std::visit (
//        [&_newContainerName, &_newQueryName] (auto &_task)
//        {
//            if constexpr (std::is_base_of_v <QueryPreparationBase, std::decay_t <decltype (_task)>>)
//            {
//                *reinterpret_cast <const void *const *> (&_task.typeMapping) = _newContainerName;
//                _task.queryName = _newQueryName;
//            }
//            else
//            {
//                REQUIRE_WITH_MESSAGE (false, "Query preparation task should inherit QueryPreparationBase.");
//            }
//        },
//        _queryPreparation);
//
//    return _queryPreparation;
//}

//void ForPreparedQuery (
//    const Reference::Test::Scenario &_scenario, const Query::Test::Storage &_containerDescriptor,
//    const Task &_queryPreparation)
//{
//    std::vector <Task> tasks;
//    bool containerCreated = false;
//    bool anyReferenceModificationExecuted = false;
//
//    for (const Reference::Test::Task &packedTask : _scenario)
//    {
//        std::visit (
//            [&tasks, &anyReferenceModificationExecuted, &containerCreated,
//                &_containerDescriptor, &_queryPreparation] (const auto &_task)
//            {
//                using TaskType = std::decay_t <decltype (_task)>;
//                if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::Create>)
//                {
//                    if (containerCreated)
//                    {
//                        tasks.emplace_back (TestQueryApiImporters::ContainerAcquisitionTaskFromSource (
//                            _containerDescriptor.sources[0u], _containerDescriptor.dataType,
//                            "TemporaryContainer"));
//                    }
//                    else
//                    {
//                        REQUIRE_WITH_MESSAGE (
//                            !anyReferenceModificationExecuted,
//                            "For simplicity container creation after reference modification is not supported, "
//                            "because it would introduce branching into test.");
//
//                        tasks += TestQueryApiImporters::ImportStorage (
//                            _containerDescriptor, "TemporaryContainer", false);
//                        containerCreated = true;
//                    }
//
//                    tasks.emplace_back (RenamePreparedQuery (_queryPreparation, "TemporaryContainer", _task.name));
//                    tasks.emplace_back (Delete <ContainerReferenceTag> {{"TemporaryContainer"}});
//                }
//                else if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::CheckStatus>)
//                {
//                    tasks.emplace_back (ContainerAcquisitionTaskToAllocationCheck (
//                        TestQueryApiImporters::ContainerAcquisitionTaskFromSource (
//                            _containerDescriptor.sources[0u], _containerDescriptor.dataType, {}),
//                        _task.hasAnyReferences));
//                }
//                else
//                {
//                    anyReferenceModificationExecuted = true;
//                    if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::Move>)
//                    {
//                        tasks.emplace_back (Move <PreparedQueryTag> {_task.sourceName, _task.targetName});
//                    }
//                    else if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::Copy>)
//                    {
//                        tasks.emplace_back (Copy <PreparedQueryTag> {_task.sourceName, _task.targetName});
//                    }
//                    else if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::Delete>)
//                    {
//                        tasks.emplace_back (Delete <PreparedQueryTag> {_task.name});
//                    }
//                }
//            },
//            packedTask);
//    }
//
//    Scenario {tasks};
//}

void ForCursor (
    const Reference::Test::Scenario &_scenario, const Query::Test::Storage &_environmentDescriptor,
    const Query::Test::Task &_query, const void *_cursorExpectedObject)
{
    std::vector <Task> tasks = TestQueryApiImporters::ImportStorage (_environmentDescriptor);
    for (const Reference::Test::Task &packedTask : _scenario)
    {
        std::visit (
            [&tasks, &_query, _cursorExpectedObject] (const auto &_task)
            {
                using TaskType = std::decay_t <decltype (_task)>;
                if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::Create>)
                {
                    std::visit (
                        [&tasks] (const auto &_query)
                        {
                            tasks.emplace_back (TestQueryApiImporters::ImportTask (_query));
                        },
                        Query::Test::ChangeQuerySourceAndCursor (_query, std::nullopt, _task.name));

                    tasks.emplace_back (CursorCheck {_task.name, _cursorExpectedObject});
                }
                else if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::CheckStatus>)
                {
                    // Cursor creation/destruction doesn't change status of prepared queries,
                    // therefore we just skip this task.
                }
                else if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::Move>)
                {
                    tasks.emplace_back (Move <CursorTag> {_task.sourceName, _task.targetName});
                    tasks.emplace_back (CursorCheck {_task.targetName, _cursorExpectedObject});
                }
                else if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::Copy>)
                {
                    tasks.emplace_back (Copy <CursorTag> {_task.sourceName, _task.targetName});
                    tasks.emplace_back (CursorCheck {_task.targetName, _cursorExpectedObject});
                }
                else if constexpr (std::is_same_v <TaskType, Reference::Test::Tasks::Delete>)
                {
                    tasks.emplace_back (Delete <CursorTag> {_task.name});
                }
            },
            packedTask);
    }

    Scenario {tasks};
}
} // namespace TestReferenceApiDrivers

Scenario::Scenario (std::vector <Task> _tasks)
    : tasks (std::move (_tasks))
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

std::vector <Task> &operator += (std::vector <Task> &first, const std::vector <Task> &second) noexcept
{
    for (const Task &task : second)
    {
        first.emplace_back (task);
    }

    return first;
}

std::vector <Task> operator + (std::vector <Task> first, const Task &_task) noexcept
{
    first.emplace_back (_task);
    return first;
}

} // namespace Emergence::Warehouse::Test