#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <StandardLayout/Field.hpp>
#include <StandardLayout/Mapping.hpp>

namespace Emergence::Query::Test
{
namespace Sources
{
struct Singleton final
{
    std::string name;
};

struct UnorderedSequence final
{
    std::string name;
};

struct Value final
{
    std::string name;
    std::vector <StandardLayout::FieldId> queriedFields;
};

struct Range final
{
    std::string name;
    StandardLayout::FieldId queriedField;
};

struct Volumetric final
{
    union SupportedValue final
    {
        SupportedValue (int8_t _value) noexcept;

        SupportedValue (int16_t _value) noexcept;

        SupportedValue (int32_t _value) noexcept;

        SupportedValue (int64_t _value) noexcept;

        SupportedValue (uint8_t _value) noexcept;

        SupportedValue (uint16_t _value) noexcept;

        SupportedValue (uint32_t _value) noexcept;

        SupportedValue (uint64_t _value) noexcept;

        SupportedValue (float _value) noexcept;

        SupportedValue (double _value) noexcept;

        int8_t int8;
        int16_t int16;
        int32_t int32;
        int64_t int64;

        uint8_t uint8;
        uint16_t uint16;
        uint32_t uint32;
        uint64_t uint64;

        float floating;
        double doubleFloating;
    };

    struct Dimension final
    {
        SupportedValue globalMin;

        StandardLayout::FieldId minField;

        SupportedValue globalMax;

        StandardLayout::FieldId maxField;
    };

    std::string name;
    std::vector <Dimension> dimensions;
};

std::ostream &operator << (std::ostream &_output, const Volumetric::SupportedValue &_value);
} // namespace Sources

using Source = std::variant <
    Sources::Singleton,
    Sources::UnorderedSequence,
    Sources::Value,
    Sources::Range,
    Sources::Volumetric>;

struct Storage final
{
    StandardLayout::Mapping dataType;
    std::vector <const void *> objectsToInsert;
    std::vector <Source> sources;
};

namespace Tasks
{
struct QueryBase
{
    std::string sourceName;
    std::string cursorName;
};

struct QuerySingletonToRead : public QueryBase
{
};

struct QuerySingletonToEdit : public QueryBase
{
};

struct QueryUnorderedSequenceToRead : public QueryBase
{
};

struct QueryUnorderedSequenceToEdit : public QueryBase
{
};

struct ValueQueryBase : public QueryBase
{
    const void *value = nullptr;
};

struct QueryValueToRead final : public ValueQueryBase
{
};

struct QueryValueToEdit final : public ValueQueryBase
{
};

struct RangeQueryBase : public QueryBase
{
    const void *minValue = nullptr;
    const void *maxValue = nullptr;
};

struct QueryRangeToRead final : public RangeQueryBase
{
};

struct QueryRangeToEdit final : public RangeQueryBase
{
};

// TODO: Rename all reversed/* queries to descending/ascending in tests, services and libraries.
struct QueryReversedRangeToRead final : public RangeQueryBase
{
};

struct QueryReversedRangeToEdit final : public RangeQueryBase
{
};

struct ShapeIntersectionQueryBase : public QueryBase
{
    std::vector <Sources::Volumetric::SupportedValue> min;
    std::vector <Sources::Volumetric::SupportedValue> max;
};

struct QueryShapeIntersectionToRead final : public ShapeIntersectionQueryBase
{
};

struct QueryShapeIntersectionToEdit final : public ShapeIntersectionQueryBase
{
};

struct RayIntersectionQueryBase : public QueryBase
{
    std::vector <Sources::Volumetric::SupportedValue> origin;
    std::vector <Sources::Volumetric::SupportedValue> direction;
    float maxDistance = std::numeric_limits <float>::max ();
};

struct QueryRayIntersectionToRead final : public RayIntersectionQueryBase
{
};

struct QueryRayIntersectionToEdit final : public RayIntersectionQueryBase
{
};

struct CursorCheck final
{
    std::string name;
    const void *expectedObject;
};

struct CursorCheckAllOrdered final
{
    std::string name;
    std::vector <const void *> expectedObjects;
};

struct CursorCheckAllUnordered final
{
    std::string name;
    std::vector <const void *> expectedObjects;
};

struct CursorEdit final
{
    std::string name;
    const void *copyFromObject;
};

struct CursorIncrement final
{
    std::string name;
};

struct CursorDeleteObject final
{
    std::string name;
};

struct CursorClose final
{
    std::string name;
};

std::ostream &operator << (std::ostream &_output, const QuerySingletonToRead &_task);

std::ostream &operator << (std::ostream &_output, const QuerySingletonToEdit &_task);

std::ostream &operator << (std::ostream &_output, const QueryUnorderedSequenceToRead &_task);

std::ostream &operator << (std::ostream &_output, const QueryUnorderedSequenceToEdit &_task);

std::ostream &operator << (std::ostream &_output, const QueryValueToRead &_task);

std::ostream &operator << (std::ostream &_output, const QueryValueToEdit &_task);

std::ostream &operator << (std::ostream &_output, const QueryRangeToRead &_task);

std::ostream &operator << (std::ostream &_output, const QueryRangeToEdit &_task);

std::ostream &operator << (std::ostream &_output, const QueryReversedRangeToRead &_task);

std::ostream &operator << (std::ostream &_output, const QueryReversedRangeToEdit &_task);

std::ostream &operator << (std::ostream &_output, const QueryShapeIntersectionToRead &_task);

std::ostream &operator << (std::ostream &_output, const QueryShapeIntersectionToEdit &_task);

std::ostream &operator << (std::ostream &_output, const QueryRayIntersectionToRead &_task);

std::ostream &operator << (std::ostream &_output, const QueryRayIntersectionToEdit &_task);

std::ostream &operator << (std::ostream &_output, const CursorCheck &_task);

std::ostream &operator << (std::ostream &_output, const CursorCheckAllOrdered &_task);

std::ostream &operator << (std::ostream &_output, const CursorCheckAllUnordered &_task);

std::ostream &operator << (std::ostream &_output, const CursorEdit &_task);

std::ostream &operator << (std::ostream &_output, const CursorIncrement &_task);

std::ostream &operator << (std::ostream &_output, const CursorDeleteObject &_task);

std::ostream &operator << (std::ostream &_output, const CursorClose &_task);
} // namespace Tasks

using Task = std::variant <
    Tasks::QuerySingletonToRead,
    Tasks::QuerySingletonToEdit,
    Tasks::QueryUnorderedSequenceToRead,
    Tasks::QueryUnorderedSequenceToEdit,
    Tasks::QueryValueToRead,
    Tasks::QueryValueToEdit,
    Tasks::QueryRangeToRead,
    Tasks::QueryRangeToEdit,
    Tasks::QueryReversedRangeToRead,
    Tasks::QueryReversedRangeToEdit,
    Tasks::QueryShapeIntersectionToRead,
    Tasks::QueryShapeIntersectionToEdit,
    Tasks::QueryRayIntersectionToRead,
    Tasks::QueryRayIntersectionToEdit,
    Tasks::CursorCheck,
    Tasks::CursorCheckAllOrdered,
    Tasks::CursorCheckAllUnordered,
    Tasks::CursorEdit,
    Tasks::CursorIncrement,
    Tasks::CursorDeleteObject,
    Tasks::CursorClose>;

struct Scenario final
{
    std::vector <Storage> storages;
    std::vector <Task> tasks;
};

/// \brief Query-type agnostic renaming is widely used in tests, therefore it was extracted to utility function.
Task ChangeQuerySourceAndCursor (
    Task _query, std::optional <std::string> _newSourceName, std::optional <std::string> _newCursorName);

Scenario RemapSources (Scenario _scenario, const std::unordered_map <std::string, std::string> &_transformation);

std::vector <Task> &operator += (std::vector <Task> &_left, const std::vector <Task> &_right);

std::ostream &operator << (std::ostream &_output, const Task &_task);
} // namespace Emergence::Query::Test