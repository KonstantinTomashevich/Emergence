#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <Query/Test/Scenario.hpp>

#include <RecordCollection/Collection.hpp>

#include <Reference/Test/ReferenceStorage.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::RecordCollection::Test
{
using namespace Query::Test::Tasks;
using namespace Reference::Test::TemplatedTasks;

struct CreateLinearRepresentation
{
    std::string name;
    StandardLayout::FieldId keyField;
};

struct CreatePointRepresentation
{
    std::string name;
    std::vector <StandardLayout::FieldId> keyFields;
};

struct CreateVolumetricRepresentation
{
    std::string name;
    std::vector <Query::Test::Sources::Volumetric::Dimension> dimensions;
};

struct DropRepresentation
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

using RepresentationReference = std::variant <
    LinearRepresentation,
    PointRepresentation,
    VolumetricRepresentation>;

using Task = std::variant <
    CreateLinearRepresentation,
    CreatePointRepresentation,
    CreateVolumetricRepresentation,
    Copy <RepresentationReference>,
    Delete <RepresentationReference>,
    DropRepresentation,
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
void CreateRepresentationsThanAllocateRecords (const Query::Test::Scenario &_scenario);

void AllocateRecordsThanCreateRepresentations (const Query::Test::Scenario &_scenario);
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
} // namespace Emergence::RecordCollection::Test