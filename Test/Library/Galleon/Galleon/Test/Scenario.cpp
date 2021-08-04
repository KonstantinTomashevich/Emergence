#include <optional>
#include <sstream>
#include <unordered_map>

#include <Galleon/CargoDeck.hpp>
#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/CursorManager.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Galleon::Test
{
using ContainerReference = std::variant <
    Handling::Handle <SingletonContainer>,
    Handling::Handle <ShortTermContainer>,
    Handling::Handle <LongTermContainer>>;

using PreparedQuery = std::variant <
    SingletonContainer::FetchQuery,
    SingletonContainer::ModifyQuery,
    ShortTermContainer::InsertQuery,
    ShortTermContainer::FetchQuery,
    ShortTermContainer::ModifyQuery,
    LongTermContainer::InsertQuery,
    LongTermContainer::FetchValueQuery,
    LongTermContainer::ModifyValueQuery,
    LongTermContainer::FetchRangeQuery,
    LongTermContainer::ModifyRangeQuery,
    LongTermContainer::FetchReversedRangeQuery,
    LongTermContainer::ModifyReversedRangeQuery,
    LongTermContainer::FetchShapeIntersectionQuery,
    LongTermContainer::ModifyShapeIntersectionQuery,
    LongTermContainer::FetchRayIntersectionQuery,
    LongTermContainer::ModifyRayIntersectionQuery>;

class ExecutionContext final
{
public:
    ExecutionContext () = default;

    void ExecuteTask (const AcquireSingletonContainer &_task);

    void ExecuteTask (const AcquireShortTermContainer &_task);

    void ExecuteTask (const AcquireLongTermContainer &_task);

    void ExecuteTask (const CopyContainerReference &_task);

    void ExecuteTask (const MoveContainerReference &_task);

    void ExecuteTask (const RemoveContainerReference &_task);

    void ExecuteTask (const PrepareSingletonFetchQuery &_task);

    void ExecuteTask (const PrepareSingletonModifyQuery &_task);

    void ExecuteTask (const PrepareShortTermInsertQuery &_task);

    void ExecuteTask (const PrepareShortTermFetchQuery &_task);

    void ExecuteTask (const PrepareShortTermModifyQuery &_task);

    void ExecuteTask (const PrepareLongTermInsertQuery &_task);

    void ExecuteTask (const PrepareLongTermFetchValueQuery &_task);

    void ExecuteTask (const PrepareLongTermModifyValueQuery &_task);

    void ExecuteTask (const PrepareLongTermFetchRangeQuery &_task);

    void ExecuteTask (const PrepareLongTermModifyRangeQuery &_task);

    void ExecuteTask (const PrepareLongTermFetchReversedRangeQuery &_task);

    void ExecuteTask (const PrepareLongTermModifyReversedRangeQuery &_task);

    void ExecuteTask (const PrepareLongTermFetchShapeIntersectionQuery &_task);

    void ExecuteTask (const PrepareLongTermModifyShapeIntersectionQuery &_task);

    void ExecuteTask (const PrepareLongTermFetchRayIntersectionQuery &_task);

    void ExecuteTask (const PrepareLongTermModifyRayIntersectionQuery &_task);

    void ExecuteTask (const CopyPreparedQuery &_task);

    void ExecuteTask (const MovePreparedQuery &_task);

    void ExecuteTask (const RemovePreparedQuery &_task);

    void ExecuteTask (const InsertObjects &_task);

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

private:
    ContainerReference &RequireContainer (const std::string &_name);

    PreparedQuery &RequireQuery (const std::string &_name);

    std::vector <RecordCollection::Collection::DimensionDescriptor> ConvertDimensions (
        const std::vector <Query::Test::Sources::Volumetric::Dimension> &_dimensions);

    template <typename QueryType>
    std::vector <uint8_t> MergeVectorsIntoQueryParameterSequence (
        const QueryType &_query,
        const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_firstVector,
        const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_secondVector) const;

    CargoDeck deck;
    std::unordered_map <std::string, ContainerReference> containers;
    std::unordered_map <std::string, PreparedQuery> queries;

    Query::Test::CursorManager <
        SingletonContainer::FetchQuery::Cursor,
        SingletonContainer::ModifyQuery::Cursor,
        ShortTermContainer::FetchQuery::Cursor,
        ShortTermContainer::ModifyQuery::Cursor,
        LongTermContainer::FetchValueQuery::Cursor,
        LongTermContainer::ModifyValueQuery::Cursor,
        LongTermContainer::FetchRangeQuery::Cursor,
        LongTermContainer::ModifyRangeQuery::Cursor,
        LongTermContainer::FetchReversedRangeQuery::Cursor,
        LongTermContainer::ModifyReversedRangeQuery::Cursor,
        LongTermContainer::FetchShapeIntersectionQuery::Cursor,
        LongTermContainer::ModifyShapeIntersectionQuery::Cursor,
        LongTermContainer::FetchRayIntersectionQuery::Cursor,
        LongTermContainer::ModifyRayIntersectionQuery::Cursor> cursors;
};

void ExecutionContext::ExecuteTask (const AcquireSingletonContainer &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.name) == containers.end (),
        "There should be no container with name \"", _task.name, "\".");

    containers.emplace (_task.name, deck.AcquireSingletonContainer (_task.mapping));
}

void ExecutionContext::ExecuteTask (const AcquireShortTermContainer &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.name) == containers.end (),
        "There should be no container with name \"", _task.name, "\".");

    containers.emplace (_task.name, deck.AcquireShortTermContainer (_task.mapping));
}

void ExecutionContext::ExecuteTask (const AcquireLongTermContainer &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.name) == containers.end (),
        "There should be no container with name \"", _task.name, "\".");

    containers.emplace (_task.name, deck.AcquireLongTermContainer (_task.mapping));
}

void ExecutionContext::ExecuteTask (const CopyContainerReference &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.targetName) == containers.end (),
        "There should be no container with name \"", _task.targetName, "\".");

    auto sourceIterator = containers.find (_task.sourceName);

    REQUIRE_WITH_MESSAGE (
        sourceIterator != containers.end (),
        "There should be container with name \"", _task.sourceName, "\".");

    containers.emplace (_task.targetName, sourceIterator->second);
}

void ExecutionContext::ExecuteTask (const MoveContainerReference &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.targetName) == containers.end (),
        "There should be no container with name \"", _task.targetName, "\".");

    auto sourceIterator = containers.find (_task.sourceName);

    REQUIRE_WITH_MESSAGE (
        sourceIterator != containers.end (),
        "There should be container with name \"", _task.sourceName, "\".");

    containers.emplace (_task.targetName, std::move (sourceIterator->second));
}

void ExecutionContext::ExecuteTask (const RemoveContainerReference &_task)
{
    auto iterator = containers.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != containers.end (), "There should be container with name \"", _task.name, "\".");
    containers.erase (iterator);
}

void ExecutionContext::ExecuteTask (const PrepareSingletonFetchQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <SingletonContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->Fetch ());
}

void ExecutionContext::ExecuteTask (const PrepareSingletonModifyQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <SingletonContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->Modify ());
}

void ExecutionContext::ExecuteTask (const PrepareShortTermInsertQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <ShortTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->Insert ());
}

void ExecutionContext::ExecuteTask (const PrepareShortTermFetchQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <ShortTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->Fetch ());
}

void ExecutionContext::ExecuteTask (const PrepareShortTermModifyQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <ShortTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->Modify ());
}

void ExecutionContext::ExecuteTask (const PrepareLongTermInsertQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->Insert ());
}

void ExecutionContext::ExecuteTask (const PrepareLongTermFetchValueQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->FetchValue (_task.keyFields));
}

void ExecutionContext::ExecuteTask (const PrepareLongTermModifyValueQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->ModifyValue (_task.keyFields));
}

void ExecutionContext::ExecuteTask (const PrepareLongTermFetchRangeQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->FetchRange (_task.keyField));
}

void ExecutionContext::ExecuteTask (const PrepareLongTermModifyRangeQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->ModifyRange (_task.keyField));
}

void ExecutionContext::ExecuteTask (const PrepareLongTermFetchReversedRangeQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->FetchReversedRange (_task.keyField));
}

void ExecutionContext::ExecuteTask (const PrepareLongTermModifyReversedRangeQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->ModifyReversedRange (_task.keyField));
}

void ExecutionContext::ExecuteTask (const PrepareLongTermFetchShapeIntersectionQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->FetchShapeIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecutionContext::ExecuteTask (const PrepareLongTermModifyShapeIntersectionQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->ModifyShapeIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecutionContext::ExecuteTask (const PrepareLongTermFetchRayIntersectionQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->FetchRayIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecutionContext::ExecuteTask (const PrepareLongTermModifyRayIntersectionQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        containers.find (_task.queryName) == containers.end (),
        "There should be no query with name \"", _task.queryName, "\".");

    auto &container = std::get <Handling::Handle <LongTermContainer>> (RequireContainer (_task.containerName));
    queries.emplace (_task.queryName, container->ModifyRayIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecutionContext::ExecuteTask (const CopyPreparedQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        queries.find (_task.targetName) == queries.end (),
        "There should be no prepared query with name \"", _task.targetName, "\".");

    auto sourceIterator = queries.find (_task.sourceName);

    REQUIRE_WITH_MESSAGE (
        sourceIterator != queries.end (),
        "There should be prepared query with name \"", _task.sourceName, "\".");

    queries.emplace (_task.targetName, sourceIterator->second);
}

void ExecutionContext::ExecuteTask (const MovePreparedQuery &_task)
{
    REQUIRE_WITH_MESSAGE (
        queries.find (_task.targetName) == queries.end (),
        "There should be no prepared query with name \"", _task.targetName, "\".");

    auto sourceIterator = queries.find (_task.sourceName);

    REQUIRE_WITH_MESSAGE (
        sourceIterator != queries.end (),
        "There should be prepared query with name \"", _task.sourceName, "\".");

    queries.emplace (_task.targetName, std::move (sourceIterator->second));
}

void ExecutionContext::ExecuteTask (const RemovePreparedQuery &_task)
{
    auto iterator = queries.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != queries.end (),
                          "There should be prepared query with name \"", _task.name, "\".");
    queries.erase (iterator);
}

void ExecutionContext::ExecuteTask (const InsertObjects &_task)
{
    std::visit (
        [&_task] (auto &_query)
        {
            using QueryType = std::decay_t <decltype (_query)>;
            if constexpr(std::is_same_v <QueryType, ShortTermContainer::InsertQuery> ||
                         std::is_same_v <QueryType, LongTermContainer::InsertQuery>)
            {
                auto cursor = _query.Execute ();
                for (const void *source : _task.copyFrom)
                {
                    void *target = ++cursor;
                    memcpy (target, source, _query.GetContainer ()->GetTypeMapping ().GetObjectSize ());
                }
            }
            else if constexpr (std::is_same_v <QueryType, SingletonContainer::ModifyQuery>)
            {
                REQUIRE (_task.copyFrom.size () == 1u);
                auto cursor = _query.Execute ();
                memcpy (*cursor, _task.copyFrom[0u],
                        _query.GetContainer ()->GetTypeMapping ().GetObjectSize ());
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Prepared query with name \"", _task.name, "\" must be insertion query.");
            }
        },
        RequireQuery (_task.name));
}

void ExecutionContext::ExecuteTask (const QueryValueToRead &_task)
{
    auto &query = std::get <LongTermContainer::FetchValueQuery> (RequireQuery (_task.sourceName));
    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (_task.value));
}

void ExecutionContext::ExecuteTask (const QueryValueToEdit &_task)
{
    auto &query = std::get <LongTermContainer::ModifyValueQuery> (RequireQuery (_task.sourceName));
    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (_task.value));
}

void ExecutionContext::ExecuteTask (const QueryRangeToRead &_task)
{
    auto &query = std::get <LongTermContainer::FetchRangeQuery> (RequireQuery (_task.sourceName));
    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (_task.minValue, _task.maxValue));
}

void ExecutionContext::ExecuteTask (const QueryRangeToEdit &_task)
{
    auto &query = std::get <LongTermContainer::ModifyRangeQuery> (RequireQuery (_task.sourceName));
    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (_task.minValue, _task.maxValue));
}

void ExecutionContext::ExecuteTask (const QueryReversedRangeToRead &_task)
{
    auto &query = std::get <LongTermContainer::FetchReversedRangeQuery> (RequireQuery (_task.sourceName));
    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (_task.minValue, _task.maxValue));
}

void ExecutionContext::ExecuteTask (const QueryReversedRangeToEdit &_task)
{
    auto &query = std::get <LongTermContainer::ModifyReversedRangeQuery> (RequireQuery (_task.sourceName));
    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (_task.minValue, _task.maxValue));
}

void ExecutionContext::ExecuteTask (const QueryShapeIntersectionToRead &_task)
{
    auto &query = std::get <LongTermContainer::FetchShapeIntersectionQuery> (RequireQuery (_task.sourceName));
    std::vector <uint8_t> sequence = MergeVectorsIntoQueryParameterSequence (query, _task.min, _task.max);

    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (&sequence[0u]));
}

void ExecutionContext::ExecuteTask (const QueryShapeIntersectionToEdit &_task)
{
    auto &query = std::get <LongTermContainer::ModifyShapeIntersectionQuery> (RequireQuery (_task.sourceName));
    std::vector <uint8_t> sequence = MergeVectorsIntoQueryParameterSequence (query, _task.min, _task.max);

    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (&sequence[0u]));
}

void ExecutionContext::ExecuteTask (const QueryRayIntersectionToRead &_task)
{
    auto &query = std::get <LongTermContainer::FetchRayIntersectionQuery> (RequireQuery (_task.sourceName));
    std::vector <uint8_t> sequence = MergeVectorsIntoQueryParameterSequence (query, _task.origin, _task.direction);

    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (&sequence[0u], _task.maxDistance));
}

void ExecutionContext::ExecuteTask (const QueryRayIntersectionToEdit &_task)
{
    auto &query = std::get <LongTermContainer::ModifyRayIntersectionQuery> (RequireQuery (_task.sourceName));
    std::vector <uint8_t> sequence = MergeVectorsIntoQueryParameterSequence (query, _task.origin, _task.direction);

    cursors.Add (_task.cursorName, query.GetContainer ()->GetTypeMapping (),
                 query.Execute (&sequence[0u], _task.maxDistance));
}

void ExecutionContext::ExecuteTask (const CursorCheck &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorCheckAllOrdered &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorCheckAllUnordered &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorEdit &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorIncrement &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorDeleteObject &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorCopy &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorMove &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorClose &_task)
{
    cursors.ExecuteTask (_task);
}

ContainerReference &ExecutionContext::RequireContainer (const std::string &_name)
{
    auto iterator = containers.find (_name);
    REQUIRE_WITH_MESSAGE (iterator != containers.end (),
                          "There should be container reference with name \"", _name, "\".");
    return iterator->second;
}

PreparedQuery &ExecutionContext::RequireQuery (const std::string &_name)
{
    auto iterator = queries.find (_name);
    REQUIRE_WITH_MESSAGE (iterator != queries.end (), "There should be prepared query with name \"", _name, "\".");
    return iterator->second;
}

std::vector <RecordCollection::Collection::DimensionDescriptor> ExecutionContext::ConvertDimensions (
    const std::vector <Query::Test::Sources::Volumetric::Dimension> &_dimensions)
{
    std::vector <RecordCollection::Collection::DimensionDescriptor> convertedDimensions;
    convertedDimensions.reserve (_dimensions.size ());

    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _dimensions)
    {
        convertedDimensions.emplace_back (
            RecordCollection::Collection::DimensionDescriptor
                {
                    &dimension.globalMin,
                    dimension.minField,
                    &dimension.globalMax,
                    dimension.maxField,
                });
    }

    return convertedDimensions;
}

template <typename QueryType>
std::vector <uint8_t>
ExecutionContext::MergeVectorsIntoQueryParameterSequence (
    const QueryType &_query,
    const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_firstVector,
    const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_secondVector) const
{
    std::size_t sequenceSize = 0u;
    std::size_t dimensionCount = 0u;

    for (auto iterator = _query.DimensionBegin (); iterator != _query.DimensionEnd (); ++iterator)
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

    for (auto iterator = _query.DimensionBegin (); iterator != _query.DimensionEnd (); ++iterator)
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

std::ostream &operator << (std::ostream &_output, const AcquireSingletonContainer &_task)
{
    return _output << "Acquire singleton container \"" << _task.name << "\" for mapping " <<
                   *reinterpret_cast <const void *const *> (&_task.mapping) << ".";
}

std::ostream &operator << (std::ostream &_output, const AcquireShortTermContainer &_task)
{
    return _output << "Acquire short term container \"" << _task.name << "\" for mapping " <<
                   *reinterpret_cast <const void *const *> (&_task.mapping) << ".";
}

std::ostream &operator << (std::ostream &_output, const AcquireLongTermContainer &_task)
{
    return _output << "Acquire long term container \"" << _task.name << "\" for mapping " <<
                   *reinterpret_cast <const void *const *> (&_task.mapping) << ".";
}

std::ostream &operator << (std::ostream &_output, const CopyContainerReference &_task)
{
    return _output << "Copy container reference \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const MoveContainerReference &_task)
{
    return _output << "Move container reference \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const RemoveContainerReference &_task)
{
    return _output << "Remove container reference \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareSingletonFetchQuery &_task)
{
    return _output << "Prepare singleton fetch query \"" << _task.queryName << "\" for container \""
                   << _task.containerName << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareSingletonModifyQuery &_task)
{
    return _output << "Prepare singleton modify query \"" << _task.queryName << "\" for container \""
                   << _task.containerName << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareShortTermInsertQuery &_task)
{
    return _output << "Prepare insert query \"" << _task.queryName << "\" for short term container \""
                   << _task.containerName << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareShortTermFetchQuery &_task)
{
    return _output << "Prepare fetch query \"" << _task.queryName << "\" for short term container \""
                   << _task.containerName << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareShortTermModifyQuery &_task)
{
    return _output << "Prepare modify query \"" << _task.queryName << "\" for short term container \""
                   << _task.containerName << "\".";
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermInsertQuery &_task)
{
    return _output << "Prepare insert query \"" << _task.queryName << "\" for long term container \""
                   << _task.containerName << "\".";
}

std::ostream &operator << (std::ostream &_output, const std::vector <StandardLayout::FieldId> &_fields)
{
    for (StandardLayout::FieldId id : _fields)
    {
        _output << " " << id;
    }

    return _output;
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermFetchValueQuery &_task)
{
    return _output << "Prepare fetch value query \"" << _task.queryName << "\" for long term container \"" <<
                   _task.containerName << "\" on fields" << _task.keyFields << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermModifyValueQuery &_task)
{
    return _output << "Prepare modify value query \"" << _task.queryName << "\" for long term container \"" <<
                   _task.containerName << "\" on fields" << _task.keyFields << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermFetchRangeQuery &_task)
{
    return _output << "Prepare fetch range query \"" << _task.queryName << "\" for long term container \"" <<
                   _task.containerName << "\" on field " << _task.keyField << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermModifyRangeQuery &_task)
{
    return _output << "Prepare modify range query \"" << _task.queryName << "\" for long term container \"" <<
                   _task.containerName << "\" on field " << _task.keyField << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermFetchReversedRangeQuery &_task)
{
    return _output << "Prepare fetch reversed range query \"" << _task.queryName << "\" for long term container \"" <<
                   _task.containerName << "\" on field " << _task.keyField << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermModifyReversedRangeQuery &_task)
{
    return _output << "Prepare modify reversed range query \"" << _task.queryName <<
                   "\" for long term container \"" << _task.containerName << "\" on field " << _task.keyField << ".";
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

std::ostream &operator << (std::ostream &_output, const PrepareLongTermFetchShapeIntersectionQuery &_task)
{
    return _output << "Prepare fetch shape intersections query \"" << _task.queryName <<
                   "\" for long term container \"" << _task.containerName << "\" on dimensions" <<
                   _task.dimensions << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermModifyShapeIntersectionQuery &_task)
{
    return _output << "Prepare modify shape intersections query \"" << _task.queryName <<
                   "\" for long term container \"" << _task.containerName << "\" on dimensions" <<
                   _task.dimensions << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermFetchRayIntersectionQuery &_task)
{
    return _output << "Prepare fetch ray intersections query \"" << _task.queryName <<
                   "\" for long term container \"" << _task.containerName << "\" on dimensions" <<
                   _task.dimensions << ".";
}

std::ostream &operator << (std::ostream &_output, const PrepareLongTermModifyRayIntersectionQuery &_task)
{
    return _output << "Prepare modify ray intersections query \"" << _task.queryName <<
                   "\" for long term container \"" << _task.containerName << "\" on dimensions" <<
                   _task.dimensions << ".";
}

std::ostream &operator << (std::ostream &_output, const CopyPreparedQuery &_task)
{
    return _output << "Copy prepared query \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const MovePreparedQuery &_task)
{
    return _output << "Move prepared query \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const RemovePreparedQuery &_task)
{
    return _output << "Remove prepared query \"" << _task.name << "\".";
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

template <typename ExternalTask>
std::optional <Task> ImportTask (const ExternalTask &_task)
{
    return _task;
}

template <>
std::optional <Task> ImportTask (const CheckIsSourceBusy &)
{
    // There is no busy-check functionality in Galleon, therefore we just ignore this task.
    return std::nullopt;
}

template <>
std::optional <Task> ImportTask (const QueryValueToRead &_task)
{
    return QueryValueToRead {{{_task.sourceName + "::Fetch", _task.cursorName}, _task.value}};
}

template <>
std::optional <Task> ImportTask (const QueryValueToEdit &_task)
{
    return QueryValueToEdit {{{_task.sourceName + "::Modify", _task.cursorName}, _task.value}};
}

template <>
std::optional <Task> ImportTask (const QueryRangeToRead &_task)
{
    return QueryRangeToRead {{{_task.sourceName + "::Fetch", _task.cursorName}, _task.minValue, _task.maxValue}};
}

template <>
std::optional <Task> ImportTask (const QueryRangeToEdit &_task)
{
    return QueryRangeToEdit {{{_task.sourceName + "::Modify", _task.cursorName}, _task.minValue, _task.maxValue}};
}

template <>
std::optional <Task> ImportTask (const QueryReversedRangeToRead &_task)
{
    return QueryReversedRangeToRead {{{_task.sourceName + "::FetchReversed", _task.cursorName},
                                         _task.minValue, _task.maxValue}};
}

template <>
std::optional <Task> ImportTask (const QueryReversedRangeToEdit &_task)
{
    return QueryReversedRangeToEdit {{{_task.sourceName + "::ModifyReversed", _task.cursorName},
                                         _task.minValue, _task.maxValue}};
}

template <>
std::optional <Task> ImportTask (const QueryShapeIntersectionToRead &_task)
{
    return QueryShapeIntersectionToRead {{{_task.sourceName + "::FetchShape", _task.cursorName},
                                              _task.min, _task.max}};
}

template <>
std::optional <Task> ImportTask (const QueryShapeIntersectionToEdit &_task)
{
    return QueryShapeIntersectionToEdit {{{_task.sourceName + "::ModifyShape", _task.cursorName},
                                              _task.min, _task.max}};
}

template <>
std::optional <Task> ImportTask (const QueryRayIntersectionToRead &_task)
{
    return QueryRayIntersectionToRead {{{_task.sourceName + "::FetchRay", _task.cursorName},
                                          _task.origin, _task.direction, _task.maxDistance}};
}

template <>
std::optional <Task> ImportTask (const QueryRayIntersectionToEdit &_task)
{
    return QueryRayIntersectionToEdit {{{_task.sourceName + "::ModifyRay", _task.cursorName},
                                        _task.origin, _task.direction, _task.maxDistance}};
}

void TestQueryApiDriver (const Query::Test::Scenario &_scenario)
{
    std::vector <Task> tasks;
    for (std::size_t index = 0u; index < _scenario.storages.size (); ++index)
    {
        const Query::Test::Storage &storage = _scenario.storages[index];
        if (!storage.sources.empty ())
        {
            const std::string containerName = std::to_string (index);

            const auto AcquisitionTaskFromSourceType =
                [&containerName, &storage] (const Query::Test::Source &_source) -> Task
                {
                    if (std::holds_alternative <Query::Test::Sources::Value> (_source) ||
                        std::holds_alternative <Query::Test::Sources::Range> (_source) ||
                        std::holds_alternative <Query::Test::Sources::Volumetric> (_source))
                    {
                        return AcquireLongTermContainer {{storage.dataType, containerName}};
                    }
                    else
                    {
                        REQUIRE_WITH_MESSAGE (false, "Unable to select container type!");
                        return AcquireSingletonContainer {{storage.dataType, containerName}};
                    }
                };

            Task containerAcquisition = AcquisitionTaskFromSourceType (storage.sources.front ());
            tasks.emplace_back (containerAcquisition);

            if (std::holds_alternative <AcquireSingletonContainer> (containerAcquisition))
            {
                REQUIRE (storage.objectsToInsert.size () == 1u);
                tasks.emplace_back (PrepareSingletonModifyQuery {{containerName, containerName + "::Init"}});
            }
            else if (std::holds_alternative <AcquireShortTermContainer> (containerAcquisition))
            {
                tasks.emplace_back (PrepareShortTermInsertQuery {{containerName, containerName + "::Init"}});
            }
            else
            {
                REQUIRE (std::holds_alternative <AcquireLongTermContainer> (containerAcquisition));
                tasks.emplace_back (PrepareLongTermInsertQuery {{containerName, containerName + "::Init"}});
            }

            tasks.emplace_back (InsertObjects {containerName + "::Init", storage.objectsToInsert});
            tasks.emplace_back (RemovePreparedQuery {containerName + "::Init"});

            for (const auto &source : storage.sources)
            {
                REQUIRE (containerAcquisition.index () == AcquisitionTaskFromSourceType (source).index ());
                std::visit (
                    [&tasks, &containerName] (const auto &_source)
                    {
                        using SourceType = std::decay_t <decltype (_source)>;

                        // For simplicity, we prepare all possible type of queries for given sources.

                        if constexpr (std::is_same_v <SourceType, Query::Test::Sources::Value>)
                        {
                            tasks.emplace_back (
                                PrepareLongTermFetchValueQuery
                                    {{containerName, _source.name + "::Fetch"}, _source.queriedFields});

                            tasks.emplace_back (
                                PrepareLongTermModifyValueQuery
                                    {{containerName, _source.name + "::Modify"}, _source.queriedFields});
                        }
                        else if constexpr (std::is_same_v <SourceType, Query::Test::Sources::Range>)
                        {
                            tasks.emplace_back (
                                PrepareLongTermFetchRangeQuery
                                    {{containerName, _source.name + "::Fetch"}, _source.queriedField});

                            tasks.emplace_back (
                                PrepareLongTermModifyRangeQuery
                                    {{containerName, _source.name + "::Modify"}, _source.queriedField});

                            tasks.emplace_back (
                                PrepareLongTermFetchReversedRangeQuery
                                    {{containerName, _source.name + "::FetchReversed"}, _source.queriedField});

                            tasks.emplace_back (
                                PrepareLongTermModifyReversedRangeQuery
                                    {{containerName, _source.name + "::ModifyReversed"}, _source.queriedField});
                        }
                        else if constexpr (std::is_same_v <SourceType, Query::Test::Sources::Volumetric>)
                        {
                            tasks.emplace_back (
                                PrepareLongTermFetchShapeIntersectionQuery
                                    {{containerName, _source.name + "::FetchShape"}, _source.dimensions});

                            tasks.emplace_back (
                                PrepareLongTermModifyShapeIntersectionQuery
                                    {{containerName, _source.name + "::ModifyShape"}, _source.dimensions});

                            tasks.emplace_back (
                                PrepareLongTermFetchRayIntersectionQuery
                                    {{containerName, _source.name + "::FetchRay"}, _source.dimensions});

                            tasks.emplace_back (
                                PrepareLongTermModifyRayIntersectionQuery
                                    {{containerName, _source.name + "::ModifyRay"}, _source.dimensions});
                        }
                    },
                    source);
            }
        }
    }

    for (const auto &task : _scenario.tasks)
    {
        std::visit (
            [&tasks] (const auto &_task)
            {
                if (std::optional <Task> task = ImportTask (_task); task.has_value ())
                {
                    tasks.emplace_back (task.value ());
                }
            },
            task);
    }

    Scenario {tasks};
}

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
} // namespace Emergence::Galleon::Test