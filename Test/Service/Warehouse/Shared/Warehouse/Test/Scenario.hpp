#pragma once

#include <cstdint>
#include <variant>

#include <Container/String.hpp>

#include <Context/Extension/ObjectStorage.hpp>

#include <Query/Test/Scenario.hpp>

#include <Reference/Test/Scenario.hpp>

#include <StandardLayout/Mapping.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::Warehouse::Test
{
using namespace Query::Test::Tasks;
using namespace Context::Extension::Tasks;

struct QueryPreparationBase
{
    StandardLayout::Mapping typeMapping;
    Container::String queryName;
};

struct PrepareFetchSingletonQuery : public QueryPreparationBase
{
};

struct PrepareModifySingletonQuery : public QueryPreparationBase
{
};

struct PrepareInsertShortTermQuery : public QueryPreparationBase
{
};

struct PrepareFetchSequenceQuery : public QueryPreparationBase
{
};

struct PrepareModifySequenceQuery : public QueryPreparationBase
{
};

struct PrepareInsertLongTermQuery : public QueryPreparationBase
{
};

struct PrepareFetchValueQuery : public QueryPreparationBase
{
    Container::Vector<StandardLayout::FieldId> keyFields;
};

struct PrepareModifyValueQuery : public QueryPreparationBase
{
    Container::Vector<StandardLayout::FieldId> keyFields;
};

struct PrepareFetchAscendingRangeQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
};

struct PrepareModifyAscendingRangeQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
};

struct PrepareFetchDescendingRangeQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
};

struct PrepareModifyDescendingRangeQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
};

struct PrepareFetchSignalQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
    std::array<uint8_t, sizeof(uint64_t)> signaledValue;
};

struct PrepareModifySignalQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
    std::array<uint8_t, sizeof(uint64_t)> signaledValue;
};

struct PrepareFetchShapeIntersectionQuery : public QueryPreparationBase
{
    Container::Vector<Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct PrepareModifyShapeIntersectionQuery : public QueryPreparationBase
{
    Container::Vector<Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct PrepareFetchRayIntersectionQuery : public QueryPreparationBase
{
    Container::Vector<Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct PrepareModifyRayIntersectionQuery : public QueryPreparationBase
{
    Container::Vector<Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct InsertObjects
{
    Container::String name;
    Container::Vector<const void *> copyFrom;
};

using Task = std::variant<PrepareFetchSingletonQuery,
                          PrepareModifySingletonQuery,
                          PrepareInsertShortTermQuery,
                          PrepareFetchSequenceQuery,
                          PrepareModifySequenceQuery,
                          PrepareInsertLongTermQuery,
                          PrepareFetchValueQuery,
                          PrepareModifyValueQuery,
                          PrepareFetchAscendingRangeQuery,
                          PrepareModifyAscendingRangeQuery,
                          PrepareFetchDescendingRangeQuery,
                          PrepareModifyDescendingRangeQuery,
                          PrepareFetchSignalQuery,
                          PrepareModifySignalQuery,
                          PrepareFetchShapeIntersectionQuery,
                          PrepareModifyShapeIntersectionQuery,
                          PrepareFetchRayIntersectionQuery,
                          PrepareModifyRayIntersectionQuery,
                          InsertObjects,
                          Move<struct PreparedQueryTag>,
                          Copy<struct PreparedQueryTag>,
                          Delete<struct PreparedQueryTag>,
                          QuerySingletonToRead,
                          QuerySingletonToEdit,
                          QueryUnorderedSequenceToRead,
                          QueryUnorderedSequenceToEdit,
                          QueryValueToRead,
                          QueryValueToEdit,
                          QueryAscendingRangeToRead,
                          QueryAscendingRangeToEdit,
                          QueryDescendingRangeToRead,
                          QueryDescendingRangeToEdit,
                          QuerySignalToRead,
                          QuerySignalToEdit,
                          QueryShapeIntersectionToRead,
                          QueryShapeIntersectionToEdit,
                          QueryRayIntersectionToRead,
                          QueryRayIntersectionToEdit,
                          CursorCheck,
                          CursorCheckAllOrdered,
                          CursorCheckAllUnordered,
                          CursorEdit,
                          CursorIncrement,
                          CursorDeleteObject,
                          Move<struct CursorTag>,
                          Copy<struct CursorTag>,
                          Delete<struct CursorTag>,
                          CursorClose>;

void TestQueryApiDriver (const Query::Test::Scenario &_scenario);

namespace TestReferenceApiDrivers
{
void ForPreparedQuery (const Reference::Test::Scenario &_scenario, const Task &_queryPreparation, const void *_object);

void ForCursor (const Reference::Test::Scenario &_scenario,
                const Container::Vector<Query::Test::Storage> &_environment,
                const Query::Test::Task &_query,
                const void *_cursorExpectedObject);
} // namespace TestReferenceApiDrivers

struct Scenario final
{
    struct Visualization final
    {
        VisualGraph::Graph registry;
        Container::Vector<VisualGraph::Graph> queries;
    };

    void Execute () const;

    [[nodiscard]] Visualization ExecuteAndVisualize () const;

    Container::Vector<Task> tasks;
};

std::ostream &operator<< (std::ostream &_output, const Scenario &_scenario);

Container::Vector<Task> &operator+= (Container::Vector<Task> &_first, const Container::Vector<Task> &_second);

Container::Vector<Task> operator+ (Container::Vector<Task> _first, const Task &_task);
} // namespace Emergence::Warehouse::Test
