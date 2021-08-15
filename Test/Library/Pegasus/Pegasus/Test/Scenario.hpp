#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <Context/Extension/ObjectStorage.hpp>

#include <Query/Test/Scenario.hpp>

#include <Reference/Test/Scenario.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Pegasus::Test
{
using namespace Query::Test::Tasks;
using namespace Context::Extension::Tasks;

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

struct CheckIsIndexCanBeDropped
{
    std::string name;
    bool expected = false;
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
    Move <struct IndexReferenceTag>,
    Copy <struct IndexReferenceTag>,
    MoveAssign <struct IndexReferenceTag>,
    CopyAssign <struct IndexReferenceTag>,
    Delete <struct IndexReferenceTag>,
    CheckIsIndexCanBeDropped,
    DropIndex,
    OpenAllocator,
    AllocateAndInit,
    CloseAllocator,
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
    Move <struct CursorTag>,
    Copy <struct CursorTag>,
    Delete <struct CursorTag>,
    CursorClose>;

namespace TestQueryApiDrivers
{
void CreateIndicesThanInsertRecords (const Query::Test::Scenario &_scenario);

void InsertRecordsThanCreateIndices (const Query::Test::Scenario &_scenario);
}

namespace ReferenceApiTestImporters
{
std::vector <Task> ForIndexReference (const Reference::Test::Scenario &_scenario, const std::string &_indexName);

std::vector <Task> ForCursor (
    const Reference::Test::Scenario &_scenario, const std::string &_indexName,
    const Query::Test::Task &_sourceQuery, const void *_expectedPointedObject);
} // namespace ReferenceApiTestImporters

class Scenario final
{
public:
    Scenario (StandardLayout::Mapping _mapping, std::vector <Task> _tasks);

private:
    friend std::ostream &operator << (std::ostream &_output, const Scenario &_seed);

    StandardLayout::Mapping mapping;
    std::vector <Task> tasks;
};

std::vector <Task> &operator += (std::vector <Task> &first, const std::vector <Task> &second) noexcept;
} // namespace Emergence::Pegasus::Test