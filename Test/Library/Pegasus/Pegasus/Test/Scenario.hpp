#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <Handling/Handle.hpp>

#include <Pegasus/Storage.hpp>

#include <Query/Test/Scenario.hpp>

#include <Reference/Test/ReferenceStorage.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Pegasus::Test
{
using namespace Query::Test::Tasks;
using namespace Reference::Test::TemplatedTasks;

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

using IndexReference = std::variant <
    Handling::Handle <HashIndex>,
    Handling::Handle <OrderedIndex>,
    Handling::Handle <VolumetricIndex>>;

using Task = std::variant <
    CreateHashIndex,
    CreateOrderedIndex,
    CreateVolumetricIndex,
    Copy <IndexReference>,
    Delete <IndexReference>,
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
    CursorDeleteObject,
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