#include <optional>
#include <sstream>
#include <unordered_map>

#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/CursorManager.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Galleon::Test
{
struct ExecutionContext final :
    public Reference::Test::ReferenceStorage <ContainerReference>,
    public Reference::Test::ReferenceStorage <PreparedQuery>,
    public Query::Test::CursorManager <
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
        LongTermContainer::ModifyRayIntersectionQuery::Cursor>
{
    ExecutionContext () = default;

    ~ExecutionContext ();

    CargoDeck deck;
};

ExecutionContext::~ExecutionContext ()
{
    cursors.clear ();
    Reference::Test::ReferenceStorage <ContainerReference>::references.clear ();
    Reference::Test::ReferenceStorage <PreparedQuery>::references.clear ();
}

std::vector <RecordCollection::Collection::DimensionDescriptor> ConvertDimensions (
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
std::vector <uint8_t> MergeVectorsIntoQueryParameterSequence (
    const QueryType &_query,
    const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_firstVector,
    const std::vector <Query::Test::Sources::Volumetric::SupportedValue> &_secondVector)
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

void ExecuteTask (ExecutionContext &_context, const AcquireSingletonContainer &_task)
{
    AddReference <ContainerReference> (_context, _task.name, _context.deck.AcquireSingletonContainer (_task.mapping));
}

void ExecuteTask (ExecutionContext &_context, const AcquireShortTermContainer &_task)
{
    AddReference <ContainerReference> (_context, _task.name, _context.deck.AcquireShortTermContainer (_task.mapping));
}

void ExecuteTask (ExecutionContext &_context, const AcquireLongTermContainer &_task)
{
    AddReference <ContainerReference> (_context, _task.name, _context.deck.AcquireLongTermContainer (_task.mapping));
}

void ExecuteTask (ExecutionContext &_context, const PrepareSingletonFetchQuery &_task)
{
    auto &container = std::get <Handling::Handle <SingletonContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->Fetch ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareSingletonModifyQuery &_task)
{
    auto &container = std::get <Handling::Handle <SingletonContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->Modify ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareShortTermInsertQuery &_task)
{
    auto &container = std::get <Handling::Handle <ShortTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->Insert ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareShortTermFetchQuery &_task)
{
    auto &container = std::get <Handling::Handle <ShortTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->Fetch ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareShortTermModifyQuery &_task)
{
    auto &container = std::get <Handling::Handle <ShortTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->Modify ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermInsertQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->Insert ());
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchValueQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->FetchValue (_task.keyFields));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyValueQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->ModifyValue (_task.keyFields));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchRangeQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->FetchRange (_task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyRangeQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->ModifyRange (_task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchReversedRangeQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->FetchReversedRange (_task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyReversedRangeQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));
    AddReference <PreparedQuery> (_context, _task.queryName, container->ModifyReversedRange (_task.keyField));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchShapeIntersectionQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));

    AddReference <PreparedQuery> (
        _context, _task.queryName, container->FetchShapeIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyShapeIntersectionQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));

    AddReference <PreparedQuery> (
        _context, _task.queryName, container->ModifyShapeIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermFetchRayIntersectionQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));

    AddReference <PreparedQuery> (
        _context, _task.queryName, container->FetchRayIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecuteTask (ExecutionContext &_context, const PrepareLongTermModifyRayIntersectionQuery &_task)
{
    auto &container = std::get <Handling::Handle <LongTermContainer>> (
        GetReference <ContainerReference> (_context, _task.containerName));

    AddReference <PreparedQuery> (
        _context, _task.queryName, container->ModifyRayIntersection (ConvertDimensions (_task.dimensions)));
}

void ExecuteTask (ExecutionContext &_context, const InsertObjects &_task)
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
        GetReference <PreparedQuery> (_context, _task.name));
}

void ExecuteTask (ExecutionContext &_context, const QuerySingletonToRead &_task)
{
    auto &query = std::get <SingletonContainer::FetchQuery> (GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QuerySingletonToEdit &_task)
{
    auto
        &query = std::get <SingletonContainer::ModifyQuery> (GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QueryUnorderedSequenceToRead &_task)
{
    auto &query = std::get <ShortTermContainer::FetchQuery> (GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QueryUnorderedSequenceToEdit &_task)
{
    auto
        &query = std::get <ShortTermContainer::ModifyQuery> (GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (), query.Execute ());
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToRead &_task)
{
    auto &query =
        std::get <LongTermContainer::FetchValueQuery> (GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryValueToEdit &_task)
{
    auto &query =
        std::get <LongTermContainer::ModifyValueQuery> (GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (_task.value));
}

void ExecuteTask (ExecutionContext &_context, const QueryRangeToRead &_task)
{
    auto &query =
        std::get <LongTermContainer::FetchRangeQuery> (GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryRangeToEdit &_task)
{
    auto &query =
        std::get <LongTermContainer::ModifyRangeQuery> (GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryReversedRangeToRead &_task)
{
    auto &query = std::get <LongTermContainer::FetchReversedRangeQuery> (
        GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryReversedRangeToEdit &_task)
{
    auto &query = std::get <LongTermContainer::ModifyReversedRangeQuery> (
        GetReference <PreparedQuery> (_context, _task.sourceName));
    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (_task.minValue, _task.maxValue));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToRead &_task)
{
    auto &query = std::get <LongTermContainer::FetchShapeIntersectionQuery> (
        GetReference <PreparedQuery> (_context, _task.sourceName));
    std::vector <uint8_t> sequence = MergeVectorsIntoQueryParameterSequence (query, _task.min, _task.max);

    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryShapeIntersectionToEdit &_task)
{
    auto
        &query = std::get <LongTermContainer::ModifyShapeIntersectionQuery> (
        GetReference <PreparedQuery> (_context, _task.sourceName));
    std::vector <uint8_t> sequence = MergeVectorsIntoQueryParameterSequence (query, _task.min, _task.max);

    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (&sequence[0u]));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToRead &_task)
{
    auto &query = std::get <LongTermContainer::FetchRayIntersectionQuery> (
        GetReference <PreparedQuery> (_context, _task.sourceName));
    std::vector <uint8_t> sequence = MergeVectorsIntoQueryParameterSequence (query, _task.origin, _task.direction);

    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (&sequence[0u], _task.maxDistance));
}

void ExecuteTask (ExecutionContext &_context, const QueryRayIntersectionToEdit &_task)
{
    auto &query = std::get <LongTermContainer::ModifyRayIntersectionQuery> (
        GetReference <PreparedQuery> (_context, _task.sourceName));
    std::vector <uint8_t> sequence = MergeVectorsIntoQueryParameterSequence (query, _task.origin, _task.direction);

    AddCursor (_context, _task.cursorName, query.GetContainer ()->GetTypeMapping (),
               query.Execute (&sequence[0u], _task.maxDistance));
}

EMERGENCE_TEST_REFERENCE_STORAGE_TASK_EXECUTION_DEDUCTION_HELPER (ExecutionContext, ContainerReference)

EMERGENCE_TEST_REFERENCE_STORAGE_TASK_EXECUTION_DEDUCTION_HELPER (ExecutionContext, PreparedQuery)

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

std::ostream &operator << (std::ostream &_output, const Copy <ContainerReference> &_task)
{
    return _output << "Copy container reference \"" << _task.source << "\" to \"" << _task.target << "\".";
}

std::ostream &operator << (std::ostream &_output, const Move <ContainerReference> &_task)
{
    return _output << "Move container reference \"" << _task.source << "\" to \"" << _task.target << "\".";
}

std::ostream &operator << (std::ostream &_output, const Delete <ContainerReference> &_task)
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

std::ostream &operator << (std::ostream &_output, const Copy <PreparedQuery> &_task)
{
    return _output << "Copy prepared query \"" << _task.source << "\" to \"" << _task.target << "\".";
}

std::ostream &operator << (std::ostream &_output, const Move <PreparedQuery> &_task)
{
    return _output << "Move prepared query \"" << _task.source << "\" to \"" << _task.target << "\".";
}

std::ostream &operator << (std::ostream &_output, const Delete <PreparedQuery> &_task)
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
std::optional <Task> ImportTask (const QuerySingletonToRead &_task)
{
    return QuerySingletonToRead {{_task.sourceName + "::Fetch", _task.cursorName}};
}

template <>
std::optional <Task> ImportTask (const QuerySingletonToEdit &_task)
{
    return QuerySingletonToEdit {{_task.sourceName + "::Modify", _task.cursorName}};
}

template <>
std::optional <Task> ImportTask (const QueryUnorderedSequenceToRead &_task)
{
    return QueryUnorderedSequenceToRead {{_task.sourceName + "::Fetch", _task.cursorName}};
}

template <>
std::optional <Task> ImportTask (const QueryUnorderedSequenceToEdit &_task)
{
    return QueryUnorderedSequenceToEdit {{_task.sourceName + "::Modify", _task.cursorName}};
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
                    if (std::holds_alternative <Query::Test::Sources::Singleton> (_source))
                    {
                        return AcquireSingletonContainer {{storage.dataType, containerName}};
                    }
                    else if (std::holds_alternative <Query::Test::Sources::UnorderedSequence> (_source))
                    {
                        return AcquireShortTermContainer {{storage.dataType, containerName}};
                    }
                    else
                    {
                        const bool isParametricSource =
                            std::holds_alternative <Query::Test::Sources::Value> (_source) ||
                            std::holds_alternative <Query::Test::Sources::Range> (_source) ||
                            std::holds_alternative <Query::Test::Sources::Volumetric> (_source);

                        REQUIRE (isParametricSource);
                        return AcquireLongTermContainer {{storage.dataType, containerName}};
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
            tasks.emplace_back (Delete <PreparedQuery> {{containerName + "::Init"}});

            for (const auto &source : storage.sources)
            {
                // Storage is always represented by container and containers
                // only support specific subsets of sources, not all of them.
                REQUIRE (containerAcquisition.index () == AcquisitionTaskFromSourceType (source).index ());

                std::visit (
                    [&tasks, &containerName] (const auto &_source)
                    {
                        using SourceType = std::decay_t <decltype (_source)>;

                        // For simplicity, we prepare all possible type of queries for given sources.

                        if constexpr (std::is_same_v <SourceType, Query::Test::Sources::Singleton>)
                        {
                            tasks.emplace_back (
                                PrepareSingletonFetchQuery {{containerName, _source.name + "::Fetch"}});

                            tasks.emplace_back (
                                PrepareSingletonModifyQuery {{containerName, _source.name + "::Modify"}});
                        }
                        else if constexpr (std::is_same_v <SourceType, Query::Test::Sources::UnorderedSequence>)
                        {
                            tasks.emplace_back (
                                PrepareShortTermFetchQuery {{containerName, _source.name + "::Fetch"}});

                            tasks.emplace_back (
                                PrepareShortTermModifyQuery {{containerName, _source.name + "::Modify"}});
                        }
                        else if constexpr (std::is_same_v <SourceType, Query::Test::Sources::Value>)
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

std::vector <Task> operator + (std::vector <Task> first, const Task &_task) noexcept
{
    first.emplace_back (_task);
    return first;
}
} // namespace Emergence::Galleon::Test