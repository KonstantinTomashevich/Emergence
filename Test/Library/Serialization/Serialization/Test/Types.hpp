#pragma once

#include <Container/MappingRegistry.hpp>
#include <Container/Vector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>

namespace Emergence::Serialization::Test
{
struct TrivialStruct final
{
    int8_t int8 = 0;
    int16_t int16 = 0;
    int32_t int32 = 0;
    int64_t int64 = 0;

    uint8_t uint8 = 0u;
    uint16_t uint16 = 0u;
    uint32_t uint32 = 0u;
    uint64_t uint64 = 0u;

    float floating = 0.0f;
    double doubleFloating = 0.0;

    std::array<uint8_t, 8u> data = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};

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

struct SimpleTestStruct final
{
    uint64_t a = 0u;
    uint64_t b = 0u;

    bool operator== (const SimpleTestStruct &_other) const noexcept = default;

    bool operator!= (const SimpleTestStruct &_other) const noexcept = default;

    struct Reflection
    {
        StandardLayout::FieldId a;
        StandardLayout::FieldId b;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct OneLevelNestingStruct final
{
    SimpleTestStruct first;
    SimpleTestStruct second;

    bool operator== (const OneLevelNestingStruct &_other) const noexcept = default;

    bool operator!= (const OneLevelNestingStruct &_other) const noexcept = default;

    struct Reflection
    {
        StandardLayout::FieldId first;
        StandardLayout::FieldId second;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct TwoLevelNestingStruct final
{
    SimpleTestStruct first;
    OneLevelNestingStruct second;

    bool operator== (const TwoLevelNestingStruct &_other) const noexcept = default;

    bool operator!= (const TwoLevelNestingStruct &_other) const noexcept = default;

    struct Reflection
    {
        StandardLayout::FieldId first;
        StandardLayout::FieldId second;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct VectorStruct final
{
    SimpleTestStruct simple;
    Container::Vector<SimpleTestStruct> vector;

    bool operator== (const VectorStruct &_other) const noexcept = default;

    bool operator!= (const VectorStruct &_other) const noexcept = default;

    struct Reflection
    {
        StandardLayout::FieldId simple;
        StandardLayout::FieldId vector;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct NestedVectorStruct final
{
    Container::Vector<VectorStruct> vector;

    bool operator== (const NestedVectorStruct &_other) const noexcept = default;

    bool operator!= (const NestedVectorStruct &_other) const noexcept = default;

    struct Reflection
    {
        StandardLayout::FieldId vector;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct PatchStruct final
{
    StandardLayout::Patch patch;

    bool operator== (const PatchStruct &_other) const noexcept;

    bool operator!= (const PatchStruct &_other) const noexcept;

    struct Reflection
    {
        StandardLayout::FieldId patch;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct VectorOfPatchesStruct final
{
    Container::Vector<PatchStruct> patches;

    bool operator== (const VectorOfPatchesStruct &_other) const noexcept = default;

    bool operator!= (const VectorOfPatchesStruct &_other) const noexcept = default;

    struct Reflection
    {
        StandardLayout::FieldId patches;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const Container::MappingRegistry &GetPatchableTypesRegistry () noexcept;
} // namespace Emergence::Serialization::Test
