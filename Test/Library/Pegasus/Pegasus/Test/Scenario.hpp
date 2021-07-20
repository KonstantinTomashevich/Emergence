#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <Query/Test/Scenario.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Pegasus::Test
{
using namespace Query::Test::Tasks;

struct CreateHashIndex
{
    std::string name;
    std::vector <StandardLayout::FieldId> indexedFields;
};

struct CreateOrderedIndex
{
    std::string name;
    StandardLayout::FieldId indexedField;
};

struct CreateVolumetricIndex
{
    std::string name;
    std::vector <Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct CopyIndexReference
{
    std::string sourceName;
    std::string targetName;
};

struct RemoveIndexReference
{
    std::string name;
};

struct DropIndex
{
    std::string name;
};

struct OpenAllocator
{
};

struct AllocateAndInit
{
    const void *copyFrom;
};

struct CloseAllocator
{
};

using Task = std::variant <
    CreateHashIndex,
    CreateOrderedIndex,
    CreateVolumetricIndex,
    CopyIndexReference,
    RemoveIndexReference,
    DropIndex,
    OpenAllocator,
    AllocateAndInit,
    CloseAllocator,
    CheckIsSourceBusy,
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
    CursorDeleteRecord,
    CursorCopy,
    CursorMove,
    CursorClose>;

namespace TestQueryApiDrivers
{
void CreateIndicesThanInsertRecords (const Query::Test::Scenario &_scenario);

void InsertRecordsThanCreateIndices (const Query::Test::Scenario &_scenario);
}

class Scenario final
{
public:
    Scenario (StandardLayout::Mapping _mapping, std::vector <Task> _tasks);

private:
    friend std::ostream &operator << (std::ostream &_output, const Scenario &_seed);

    StandardLayout::Mapping mapping;
    std::vector <Task> tasks;
};

std::vector <Task> operator + (std::vector <Task> first, const std::vector <Task> &second) noexcept;
} // namespace Emergence::Pegasus::Test