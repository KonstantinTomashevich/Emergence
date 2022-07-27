#pragma once

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Serialization::Test
{
struct TrivialStruct final
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

    std::array<uint8_t, 8u> data;

    bool operator== (const TrivialStruct &_other) const noexcept = default;

    bool operator!= (const TrivialStruct &_other) const noexcept = default;

    struct Reflection
    {
        StandardLayout::FieldId int8;
        StandardLayout::FieldId int16;
        StandardLayout::FieldId int32;
        StandardLayout::FieldId int64;

        StandardLayout::FieldId uint8;
        StandardLayout::FieldId uint16;
        StandardLayout::FieldId uint32;
        StandardLayout::FieldId uint64;

        StandardLayout::FieldId floating;
        StandardLayout::FieldId doubleFloating;

        StandardLayout::FieldId data;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct NonTrivialStruct final
{
    static constexpr uint8_t ALIVE_OFFSET = 0u;
    static constexpr uint8_t POISONED_OFFSET = 1u;
    static constexpr uint8_t STUNNED_OFFSET = 2u;

    uint8_t flags = 0u;
    std::array<char, 32u> string {'\0'};
    Memory::UniqueString uniqueString {"Hello, world!"};

    bool operator== (const NonTrivialStruct &_other) const noexcept;

    bool operator!= (const NonTrivialStruct &_other) const noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId string;
        StandardLayout::FieldId uniqueString;
        StandardLayout::FieldId alive;
        StandardLayout::FieldId poisoned;
        StandardLayout::FieldId stunned;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct UnionStruct final
{
    UnionStruct () noexcept = default;

    uint64_t type = 0u;
    union
    {
        struct
        {
            float x;
            float y;
        };

        struct
        {
            uint64_t m;
            uint64_t n;
        };
    };

    bool operator== (const UnionStruct &_other) const noexcept;

    bool operator!= (const UnionStruct &_other) const noexcept;

    struct Reflection
    {
        StandardLayout::FieldId type;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId m;
        StandardLayout::FieldId n;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Serialization::Test
