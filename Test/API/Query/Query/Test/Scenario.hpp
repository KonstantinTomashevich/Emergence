#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <StandardLayout/Field.hpp>
#include <StandardLayout/Mapping.hpp>

namespace Emergence::Query::Test
{
namespace Sources
{
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
struct CheckIsSourceBusy
{
    std::string name;
    bool expectedValue = false;
};

struct QueryBase
{
    std::string sourceName;
    std::string cursorName;
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

struct CursorDeleteRecord final
{
    std::string name;
};

struct CursorCopy final
{
    std::string sourceName;
    std::string targetName;
};

struct CursorMove final
{
    std::string sourceName;
    std::string targetName;
};

struct CursorClose final
{
    std::string name;
};

std::ostream &operator << (std::ostream &_output, const Tasks::CheckIsSourceBusy &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryValueToRead &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryValueToEdit &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryRangeToRead &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryRangeToEdit &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryReversedRangeToRead &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryReversedRangeToEdit &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryShapeIntersectionToRead &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryShapeIntersectionToEdit &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryRayIntersectionToRead &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::QueryRayIntersectionToEdit &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::CursorCheck &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::CursorCheckAllOrdered &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::CursorCheckAllUnordered &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::CursorEdit &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::CursorIncrement &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::CursorDeleteRecord &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::CursorCopy &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::CursorMove &_task);

std::ostream &operator << (std::ostream &_output, const Tasks::CursorClose &_task);
} // namespace Tasks

using Task = std::variant <
    Tasks::CheckIsSourceBusy,
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
    Tasks::CursorDeleteRecord,
    Tasks::CursorCopy,
    Tasks::CursorMove,
    Tasks::CursorClose>;

struct Scenario final
{
    std::vector <Storage> storages;
    std::vector <Task> tasks;
};

std::vector <Task> &operator += (std::vector <Task> &_left, const std::vector <Task> &_right);

std::ostream &operator << (std::ostream &_output, const Task &_task);
} // namespace Emergence::Query::Test