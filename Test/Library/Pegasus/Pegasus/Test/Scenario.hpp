#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Pegasus::Test
{
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

struct DimensionDescriptor
{
    const void *globalMinBorderValue;
    StandardLayout::FieldId minBorderField;

    const void *globalMaxBorderValue;
    StandardLayout::FieldId maxBorderField;
};

struct CreateVolumetricIndex
{
    std::string name;
    std::vector <DimensionDescriptor> dimensions;
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

struct CheckIndexCanBeDropped
{
    std::string name;
    bool expectedResult;
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

struct IndexLookupBase
{
    std::string indexName;
    std::string cursorName;
};

struct HashIndexLookupBase : public IndexLookupBase
{
    const void *request;
};

struct HashIndexLookupToRead : public HashIndexLookupBase
{
};

struct HashIndexLookupToEdit : public HashIndexLookupBase
{
};

struct OrderedIndexLookupBase : public IndexLookupBase
{
    const void *minValue;
    const void *maxValue;
};

struct OrderedIndexLookupToRead : public OrderedIndexLookupBase
{
};

struct OrderedIndexLookupToEdit : public OrderedIndexLookupBase
{
};

struct OrderedIndexLookupToReadReversed : public OrderedIndexLookupBase
{
};

struct OrderedIndexLookupToEditReversed : public OrderedIndexLookupBase
{
};

struct VolumetricIndexShapeIntersectionLookupBase : public IndexLookupBase
{
    // TODO: Should this be reworked into Pegasus/RecordCollection-like format?
    //       I think it shouldn't, because it'll transferred into query common tests.
    std::vector <const void *> min;
    std::vector <const void *> max;
};

struct VolumetricIndexShapeIntersectionLookupToRead :
    public VolumetricIndexShapeIntersectionLookupBase
{
};

struct VolumetricIndexShapeIntersectionLookupToEdit :
    public VolumetricIndexShapeIntersectionLookupBase
{
};

struct VolumetricIndexRayIntersectionLookupBase : public IndexLookupBase
{
    // TODO: Should this be reworked into Pegasus/RecordCollection-like format?
    //       I think it shouldn't, because it'll transferred into query common tests.
    std::vector <const void *> origin;
    std::vector <const void *> direction;
};

struct VolumetricIndexRayIntersectionLookupToRead :
    public VolumetricIndexRayIntersectionLookupBase
{
};

struct VolumetricIndexRayIntersectionLookupToEdit :
    public VolumetricIndexRayIntersectionLookupBase
{
};

struct CursorCheck
{
    std::string name;
    const void *expectedRecord;
};

struct CursorCheckAllOrdered
{
    std::string name;
    std::vector <const void *> expectedRecords;
};

struct CursorCheckAllUnordered
{
    std::string name;
    std::vector <const void *> expectedRecords;
};

struct CursorEdit
{
    std::string name;
    const void *copyFrom;
};

struct CursorIncrement
{
    std::string name;
};

struct CursorDeleteRecord
{
    std::string name;
};

struct CopyCursor
{
    std::string sourceName;
    std::string targetName;
};

struct MoveCursor
{
    std::string sourceName;
    std::string targetName;
};

struct CloseCursor
{
    std::string name;
};

using Task = std::variant <
    CreateHashIndex,
    CreateOrderedIndex,
    CreateVolumetricIndex,
    CopyIndexReference,
    RemoveIndexReference,
    CheckIndexCanBeDropped,
    DropIndex,
    OpenAllocator,
    AllocateAndInit,
    CloseAllocator,
    HashIndexLookupToRead,
    HashIndexLookupToEdit,
    OrderedIndexLookupToRead,
    OrderedIndexLookupToEdit,
    OrderedIndexLookupToReadReversed,
    OrderedIndexLookupToEditReversed,
    VolumetricIndexShapeIntersectionLookupToRead,
    VolumetricIndexShapeIntersectionLookupToEdit,
    VolumetricIndexRayIntersectionLookupToRead,
    VolumetricIndexRayIntersectionLookupToEdit,
    CursorCheck,
    CursorCheckAllOrdered,
    CursorCheckAllUnordered,
    CursorEdit,
    CursorIncrement,
    CursorDeleteRecord,
    CopyCursor,
    MoveCursor,
    CloseCursor>;

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