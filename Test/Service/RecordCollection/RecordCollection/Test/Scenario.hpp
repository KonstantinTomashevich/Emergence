#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <Context/Extension/ObjectStorage.hpp>

#include <Query/Test/Scenario.hpp>

#include <Reference/Test/Scenario.hpp>

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

struct CheckIsRepresentationCanBeDropped
{
    std::string name;
    bool expected = false;
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
    Move <struct RepresentationReferenceTag>,
    Copy <struct RepresentationReferenceTag>,
    MoveAssign <struct RepresentationReferenceTag>,
    CopyAssign <struct RepresentationReferenceTag>,
    Delete <struct RepresentationReferenceTag>,
    CheckIsRepresentationCanBeDropped,
    DropRepresentation,
    OpenAllocator,
    AllocateAndInit,
    CloseAllocator,
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
    Move <struct CursorTag>,
    Copy <struct CursorTag>,
    Delete <struct CursorTag>,
    CursorClose>;

namespace TestQueryApiDrivers
{
void CreateRepresentationsThanAllocateRecords (const Query::Test::Scenario &_scenario);

void AllocateRecordsThanCreateRepresentations (const Query::Test::Scenario &_scenario);
} // namespace TestQueryApiDrivers

namespace ReferenceApiTestImporters
{
void ForRepresentationReference (const Reference::Test::Scenario &_scenario, const Query::Test::Storage &_storage);

void ForCursor (
    const Reference::Test::Scenario &_scenario, const Query::Test::Storage &_storage,
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
} // namespace Emergence::RecordCollection::Test