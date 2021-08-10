#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <Context/Extension/ObjectStorage.hpp>

#include <Query/Test/Scenario.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::RecordCollection::Test
{
using namespace Query::Test::Tasks;
using namespace Context::Extension::Tasks;

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

using Task = std::variant <
    CreateLinearRepresentation,
    CreatePointRepresentation,
    CreateVolumetricRepresentation,
    Copy <struct RepresentationReferenceTag>,
    Delete <struct RepresentationReferenceTag>,
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