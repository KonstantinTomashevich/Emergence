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
struct SourceBase
{
    std::string name;
    StandardLayout::Mapping dataType;
    std::vector <const void *> objectsToInsert;
};

struct Value final : public SourceBase
{
    std::vector <StandardLayout::FieldId> queriedFields;
};

struct Range final : public SourceBase
{
    StandardLayout::FieldId queriedField;
};

struct Volumetric final : public SourceBase
{
    union SupportedValue final
    {
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
        StandardLayout::FieldId minField;

        SupportedValue globalMin;

        StandardLayout::FieldId maxField;

        SupportedValue globalMax;
    };

    std::vector <Dimension> dimensions;
};
} // namespace Sources

using Source = std::variant <
    Sources::Value,
    Sources::Range,
    Sources::Volumetric>;

namespace Tasks
{
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
} // namespace Tasks

using Task = std::variant <
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
    std::vector <Source> requiredSources;
    std::vector <Task> tasks;
};
} // namespace Emergence::Query::Test