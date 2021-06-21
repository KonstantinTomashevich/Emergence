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

    void Execute () const;

private:
    friend std::ostream &operator << (std::ostream &_output, const Scenario &_seed);

    StandardLayout::Mapping mapping;
    std::vector <Task> tasks;
};

std::vector <Task> operator + (std::vector <Task> first, const std::vector <Task> &second) noexcept;
} // namespace Emergence::Pegasus::Test