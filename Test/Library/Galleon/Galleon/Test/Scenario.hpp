#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <Query/Test/Scenario.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Galleon::Test
{
using namespace Query::Test::Tasks;

struct ContainerAcquisitionBase
{
    StandardLayout::Mapping mapping;
    std::string name;
};

struct AcquireSingletonContainer : public ContainerAcquisitionBase
{
};

struct AcquireShortTermContainer : public ContainerAcquisitionBase
{
};

struct AcquireLongTermContainer : public ContainerAcquisitionBase
{
};

struct CopyContainerReference
{
    std::string sourceName;
    std::string targetName;
};

struct MoveContainerReference
{
    std::string sourceName;
    std::string targetName;
};

struct RemoveContainerReference
{
    std::string name;
};

struct QueryPreparationBase
{
    std::string containerName;
    std::string queryName;
};

struct PrepareSingletonFetchQuery : public QueryPreparationBase
{
};

struct PrepareSingletonModifyQuery : public QueryPreparationBase
{
};

struct PrepareShortTermInsertQuery : public QueryPreparationBase
{
};

struct PrepareShortTermFetchQuery : public QueryPreparationBase
{
};

struct PrepareShortTermModifyQuery : public QueryPreparationBase
{
};

struct PrepareLongTermInsertQuery : public QueryPreparationBase
{
};

struct PrepareLongTermFetchValueQuery : public QueryPreparationBase
{
    std::vector <StandardLayout::FieldId> keyFields;
};

struct PrepareLongTermModifyValueQuery : public QueryPreparationBase
{
    std::vector <StandardLayout::FieldId> keyFields;
};

struct PrepareLongTermFetchRangeQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
};

struct PrepareLongTermModifyRangeQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
};

struct PrepareLongTermFetchReversedRangeQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
};

struct PrepareLongTermModifyReversedRangeQuery : public QueryPreparationBase
{
    StandardLayout::FieldId keyField = 0u;
};

struct PrepareLongTermFetchShapeIntersectionQuery : public QueryPreparationBase
{
    std::vector <Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct PrepareLongTermModifyShapeIntersectionQuery : public QueryPreparationBase
{
    std::vector <Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct PrepareLongTermFetchRayIntersectionQuery : public QueryPreparationBase
{
    std::vector <Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct PrepareLongTermModifyRayIntersectionQuery : public QueryPreparationBase
{
    std::vector <Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct CopyPreparedQuery
{
    std::string sourceName;
    std::string targetName;
};

struct MovePreparedQuery
{
    std::string sourceName;
    std::string targetName;
};

struct RemovePreparedQuery
{
    std::string name;
};

struct InsertObjects
{
    std::string name;
    std::vector <const void *> copyFrom;
};

using Task = std::variant <
    AcquireSingletonContainer,
    AcquireShortTermContainer,
    AcquireLongTermContainer,
    CopyContainerReference,
    MoveContainerReference,
    RemoveContainerReference,
    PrepareSingletonFetchQuery,
    PrepareSingletonModifyQuery,
    PrepareShortTermInsertQuery,
    PrepareShortTermFetchQuery,
    PrepareShortTermModifyQuery,
    PrepareLongTermInsertQuery,
    PrepareLongTermFetchValueQuery,
    PrepareLongTermModifyValueQuery,
    PrepareLongTermFetchRangeQuery,
    PrepareLongTermModifyRangeQuery,
    PrepareLongTermFetchReversedRangeQuery,
    PrepareLongTermModifyReversedRangeQuery,
    PrepareLongTermFetchShapeIntersectionQuery,
    PrepareLongTermModifyShapeIntersectionQuery,
    PrepareLongTermFetchRayIntersectionQuery,
    PrepareLongTermModifyRayIntersectionQuery,
    CopyPreparedQuery,
    MovePreparedQuery,
    RemovePreparedQuery,
    InsertObjects,
    QueryValueToRead,
    QueryValueToEdit,
    QueryRangeToRead,
    QueryRangeToEdit,
    QueryReversedRangeToRead,
    QueryReversedRangeToEdit,
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
    CursorCopy,
    CursorMove,
    CursorClose>;

void TestQueryApiDriver (const Query::Test::Scenario &_scenario);

class Scenario final
{
public:
    explicit Scenario (std::vector <Task> _tasks);

private:
    friend std::ostream &operator << (std::ostream &_output, const Scenario &_seed);

    std::vector <Task> tasks;
};

std::vector <Task> operator + (std::vector <Task> first, const std::vector <Task> &second) noexcept;
} // namespace Emergence::Galleon::Test