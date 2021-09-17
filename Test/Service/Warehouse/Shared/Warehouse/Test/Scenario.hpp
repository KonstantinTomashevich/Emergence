#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

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
    std::string queryName;
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
    std::vector<StandardLayout::FieldId> keyFields;
};

struct PrepareModifyValueQuery : public QueryPreparationBase
{
    std::vector<StandardLayout::FieldId> keyFields;
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

struct PrepareFetchShapeIntersectionQuery : public QueryPreparationBase
{
    std::vector<Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct PrepareModifyShapeIntersectionQuery : public QueryPreparationBase
{
    std::vector<Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct PrepareFetchRayIntersectionQuery : public QueryPreparationBase
{
    std::vector<Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct PrepareModifyRayIntersectionQuery : public QueryPreparationBase
{
    std::vector<Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct InsertObjects
{
    std::string name;
    std::vector<const void *> copyFrom;
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
                const std::vector<Query::Test::Storage> &_environment,
                const Query::Test::Task &_query,
                const void *_cursorExpectedObject);
} // namespace TestReferenceApiDrivers

struct Scenario final
{
    struct Visualization final
    {
        VisualGraph::Graph registry;
        std::vector<VisualGraph::Graph> queries;
    };

    void Execute () const noexcept;

    Visualization ExecuteAndVisualize () const noexcept;

    std::vector<Task> tasks;
};

std::ostream &operator<< (std::ostream &_output, const Scenario &_scenario);

std::vector<Task> &operator+= (std::vector<Task> &first, const std::vector<Task> &second) noexcept;

std::vector<Task> operator+ (std::vector<Task> first, const Task &_task) noexcept;
} // namespace Emergence::Warehouse::Test