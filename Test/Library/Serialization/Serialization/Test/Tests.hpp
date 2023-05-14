#pragma once

#include <Serialization/Test/Types.hpp>

#define OBJECT_SERIALIZATION_TESTS(Executor)                                                                           \
    TEST_CASE (Trivial)                                                                                                \
    {                                                                                                                  \
        Executor (TrivialStruct {                                                                                      \
            -19, 163, -2937, 1123, 21u, 784u, 17274u, 18274u, 1.20338f, 5647.385639, {11u, 79u, 54u, 121u}});          \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (NonTrivial)                                                                                             \
    {                                                                                                                  \
        Executor (NonTrivialStruct {                                                                                   \
            (1u << NonTrivialStruct::ALIVE_OFFSET) | (1u << NonTrivialStruct::STUNNED_OFFSET),                         \
            {"Let's test this code!\0"},                                                                               \
            Emergence::Memory::UniqueString {"For glory and gold!"},                                                   \
        });                                                                                                            \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (Union)                                                                                                  \
    {                                                                                                                  \
        UnionStruct first;                                                                                             \
        first.type = 0u;                                                                                               \
        first.x = 1.647f;                                                                                              \
        first.y = 173.129337f;                                                                                         \
        Executor (first);                                                                                              \
                                                                                                                       \
        UnionStruct second;                                                                                            \
        second.type = 1u;                                                                                              \
        second.m = 172947923u;                                                                                         \
        second.n = 123838471u;                                                                                         \
        Executor (second);                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (InplaceVector)                                                                                          \
    {                                                                                                                  \
        Emergence::Container::InplaceVector<UnionStruct, 4u> vector;                                                   \
        UnionStruct &first = vector.EmplaceBack ();                                                                    \
        first.type = 0u;                                                                                               \
        first.x = 1.647f;                                                                                              \
        first.y = 173.129337f;                                                                                         \
                                                                                                                       \
        UnionStruct &second = vector.EmplaceBack ();                                                                   \
        second.type = 1u;                                                                                              \
        second.m = 172947923u;                                                                                         \
        second.n = 123838471u;                                                                                         \
        Executor (vector);                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (SimpleTestStruct)                                                                                       \
    {                                                                                                                  \
        SimpleTestStruct value {13u, 197u};                                                                            \
        Executor (value);                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (OneLevelNestingStruct)                                                                                  \
    {                                                                                                                  \
        OneLevelNestingStruct value {{13u, 197u}, {1738u, 219874132u}};                                                \
        Executor (value);                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (TwoLevelNestingStruct)                                                                                  \
    {                                                                                                                  \
        TwoLevelNestingStruct value {{13456u, 23786234u}, {{13u, 197u}, {1738u, 219874132u}}};                         \
        Executor (value);                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (VectorStruct)                                                                                           \
    {                                                                                                                  \
        VectorStruct value {{13456u, 23786234u}, {}};                                                                  \
        value.vector.emplace_back () = {13u, 19u};                                                                     \
        value.vector.emplace_back () = {18376u, 1726u};                                                                \
        value.vector.emplace_back () = {99u, 276u};                                                                    \
        value.vector.emplace_back () = {1726489u, 1725648u};                                                           \
        Executor (value);                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (NestedVectorStruct)                                                                                     \
    {                                                                                                                  \
        NestedVectorStruct value;                                                                                      \
                                                                                                                       \
        VectorStruct firstVector {{13456u, 23786234u}, {}};                                                            \
        firstVector.vector.emplace_back () = {13u, 19u};                                                               \
        firstVector.vector.emplace_back () = {18376u, 1726u};                                                          \
        value.vector.emplace_back (std::move (firstVector));                                                           \
                                                                                                                       \
        VectorStruct secondVector {{16u, 234u}, {}};                                                                   \
        secondVector.vector.emplace_back () = {1323u, 191313u};                                                        \
        secondVector.vector.emplace_back () = {186u, 176u};                                                            \
        value.vector.emplace_back (std::move (secondVector));                                                          \
                                                                                                                       \
        Executor (value);                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (SimpleTestStructPatch)                                                                                  \
    {                                                                                                                  \
        const SimpleTestStruct first {13u, 179u};                                                                      \
        const SimpleTestStruct second {13u, 189u};                                                                     \
        PatchStruct value {Emergence::StandardLayout::PatchBuilder::FromDifference (                                   \
            SimpleTestStruct::Reflect ().mapping, &first, &second)};                                                   \
                                                                                                                       \
        Executor (value);                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (OneLevelNestingStructPatch)                                                                             \
    {                                                                                                                  \
        const OneLevelNestingStruct first {{13u, 197u}, {1738u, 219874132u}};                                          \
        const OneLevelNestingStruct second {{14u, 197u}, {1739u, 219874232u}};                                         \
        PatchStruct value {Emergence::StandardLayout::PatchBuilder::FromDifference (                                   \
            OneLevelNestingStruct::Reflect ().mapping, &first, &second)};                                              \
                                                                                                                       \
        Executor (value);                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (TwoLevelNestingStructPatch)                                                                             \
    {                                                                                                                  \
        const TwoLevelNestingStruct first {{13456u, 23786234u}, {{13u, 197u}, {1738u, 219874132u}}};                   \
        const TwoLevelNestingStruct second {{13456u, 23786234u}, {{13u, 196u}, {1731u, 219874139u}}};                  \
        PatchStruct value {Emergence::StandardLayout::PatchBuilder::FromDifference (                                   \
            TwoLevelNestingStruct::Reflect ().mapping, &first, &second)};                                              \
                                                                                                                       \
        Executor (value);                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (VectorOfPatchesStruct)                                                                                  \
    {                                                                                                                  \
        VectorOfPatchesStruct value;                                                                                   \
                                                                                                                       \
        {                                                                                                              \
            const SimpleTestStruct first {13u, 179u};                                                                  \
            const SimpleTestStruct second {13u, 189u};                                                                 \
            value.patches.emplace_back () = PatchStruct {Emergence::StandardLayout::PatchBuilder::FromDifference (     \
                SimpleTestStruct::Reflect ().mapping, &first, &second)};                                               \
        }                                                                                                              \
                                                                                                                       \
        {                                                                                                              \
            const OneLevelNestingStruct first {{13u, 197u}, {1738u, 219874132u}};                                      \
            const OneLevelNestingStruct second {{14u, 197u}, {1739u, 219874232u}};                                     \
            value.patches.emplace_back () = PatchStruct {Emergence::StandardLayout::PatchBuilder::FromDifference (     \
                OneLevelNestingStruct::Reflect ().mapping, &first, &second)};                                          \
        }                                                                                                              \
                                                                                                                       \
        {                                                                                                              \
            const TwoLevelNestingStruct first {{13456u, 23786234u}, {{13u, 197u}, {1738u, 219874132u}}};               \
            const TwoLevelNestingStruct second {{13456u, 23786234u}, {{13u, 196u}, {1731u, 219874139u}}};              \
            value.patches.emplace_back () = PatchStruct {Emergence::StandardLayout::PatchBuilder::FromDifference (     \
                TwoLevelNestingStruct::Reflect ().mapping, &first, &second)};                                          \
        }                                                                                                              \
                                                                                                                       \
        Executor (value);                                                                                              \
    }
