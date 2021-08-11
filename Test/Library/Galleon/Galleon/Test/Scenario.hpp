#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <Context/Extension/ObjectStorage.hpp>

#include <Query/Test/Scenario.hpp>

#include <Reference/Test/Scenario.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Galleon::Test
{
using namespace Query::Test::Tasks;
using namespace Context::Extension::Tasks;

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

struct InsertObjects
{
    std::string name;
    std::vector <const void *> copyFrom;
};

using Task = std::variant <
    AcquireSingletonContainer,
    AcquireShortTermContainer,
    AcquireLongTermContainer,
    Move <struct ContainerReferenceTag>,
    Copy <struct ContainerReferenceTag>,
    MoveAssign <struct ContainerReferenceTag>,
    CopyAssign <struct ContainerReferenceTag>,
    Delete <struct ContainerReferenceTag>,
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
    Move <struct PreparedQueryTag>,
    Copy <struct PreparedQueryTag>,
    Delete <struct PreparedQueryTag>,
    InsertObjects,
    QuerySingletonToRead,
    QuerySingletonToEdit,
    QueryUnorderedSequenceToRead,
    QueryUnorderedSequenceToEdit,
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

namespace TestReferenceApiImporters
{
Task ForContainerReference (const Reference::Test::Task &_task);

Task ForPreparedQuery (const Reference::Test::Task &_task);
} // namespace TestReferenceApiImporters

class Scenario final
{
public:
    explicit Scenario (std::vector <Task> _tasks);

private:
    friend std::ostream &operator << (std::ostream &_output, const Scenario &_seed);

    std::vector <Task> tasks;
};

std::vector <Task> &operator += (std::vector <Task> &first, const std::vector <Task> &second) noexcept;

std::vector <Task> operator + (std::vector <Task> first, const Task &_task) noexcept;
} // namespace Emergence::Galleon::Test