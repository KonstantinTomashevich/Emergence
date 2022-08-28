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
    }

#define PATCH_SERIALIZATION_TESTS(Executor)                                                                            \
    TEST_CASE (Trivial)                                                                                                \
    {                                                                                                                  \
        /* NOLINTNEXTLINE(bugprone-macro-parentheses): It's impossible to use parenthesis here. */                     \
        Executor<TrivialStruct> (/*                                                                                    \
                                    Because array data is registered as block which is not                             \
                                    supported by patches, we are filling it with zeros.                                \
                                  */                                                                                   \
                                 {-19,                                                                                 \
                                  163,                                                                                 \
                                  -2937,                                                                               \
                                  1123,                                                                                \
                                  21u,                                                                                 \
                                  784u,                                                                                \
                                  17274u,                                                                              \
                                  18274u,                                                                              \
                                  1.20338f,                                                                            \
                                  5647.385639,                                                                         \
                                  {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}},                                                   \
                                 {-20,                                                                                 \
                                  164,                                                                                 \
                                  -2936,                                                                               \
                                  4512,                                                                                \
                                  17u,                                                                                 \
                                  783u,                                                                                \
                                  26172u,                                                                              \
                                  18271u,                                                                              \
                                  1.20339f,                                                                            \
                                  5648.385639,                                                                         \
                                  {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}});                                                  \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (NonTrivial)                                                                                             \
    {                                                                                                                  \
        /* NOLINTNEXTLINE(bugprone-macro-parentheses): It's impossible to use parenthesis here. */                     \
        Executor<NonTrivialStruct> (                                                                                   \
            {                                                                                                          \
                (1u << NonTrivialStruct::ALIVE_OFFSET) | (1u << NonTrivialStruct::STUNNED_OFFSET),                     \
                                                                                                                       \
                /* Strings are zeroed because they are not supported by patches. */                                    \
                {"\0"},                                                                                                \
                Emergence::Memory::UniqueString {"For glory and gold!"},                                               \
            },                                                                                                         \
            {                                                                                                          \
                (1u << NonTrivialStruct::ALIVE_OFFSET),                                                                \
                {"\0"},                                                                                                \
                Emergence::Memory::UniqueString {"Hello, world!"},                                                     \
            });                                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    TEST_CASE (UnionPatch)                                                                                             \
    {                                                                                                                  \
        UnionStruct first;                                                                                             \
        first.type = 0u;                                                                                               \
        first.x = 1.647f;                                                                                              \
        first.y = 173.129337f;                                                                                         \
                                                                                                                       \
        UnionStruct second;                                                                                            \
        second.type = 1u;                                                                                              \
        second.m = 172947923u;                                                                                         \
        second.n = 123838471u;                                                                                         \
        Executor (first, second);                                                                                      \
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
        Executor (Emergence::Container::InplaceVector<UnionStruct, 4u> {}, vector);                                    \
    }

#define PATCH_BUNDLE_SERIALIZATION_TESTS(SerializeAndDeserialize)                                                      \
    TEST_CASE (DifferentTypes)                                                                                         \
    {                                                                                                                  \
        const TrivialStruct trivialStructInitial;                                                                      \
        const NonTrivialStruct nonTrivialStructInitial;                                                                \
        const UnionStruct unionStructInitial {};                                                                       \
                                                                                                                       \
        TrivialStruct trivialStructChanged;                                                                            \
        trivialStructChanged.int8 = -32;                                                                               \
        trivialStructChanged.uint32 = 1537;                                                                            \
                                                                                                                       \
        NonTrivialStruct nonTrivialStructChanged;                                                                      \
        nonTrivialStructChanged.flags = 1u << NonTrivialStruct::ALIVE_OFFSET;                                          \
        nonTrivialStructChanged.uniqueString = Emergence::Memory::UniqueString {"For honor!"};                         \
                                                                                                                       \
        UnionStruct unionStructChanged;                                                                                \
        unionStructChanged.type = 1u;                                                                                  \
        unionStructChanged.m = 4u;                                                                                     \
        unionStructChanged.n = 17u;                                                                                    \
                                                                                                                       \
        Emergence::Container::Vector<Emergence::StandardLayout::Patch> patches;                                        \
        SerializeAndDeserialize (                                                                                      \
            {                                                                                                          \
                Emergence::StandardLayout::PatchBuilder::FromDifference (                                              \
                    NonTrivialStruct::Reflect ().mapping, &nonTrivialStructChanged, &nonTrivialStructInitial),         \
                Emergence::StandardLayout::PatchBuilder::FromDifference (                                              \
                    TrivialStruct::Reflect ().mapping, &trivialStructChanged, &trivialStructInitial),                  \
                Emergence::StandardLayout::PatchBuilder::FromDifference (UnionStruct::Reflect ().mapping,              \
                                                                         &unionStructChanged, &unionStructInitial),    \
            },                                                                                                         \
            patches);                                                                                                  \
                                                                                                                       \
        REQUIRE_EQUAL (patches.size (), 3u);                                                                           \
        CHECK_EQUAL (patches[0u].GetTypeMapping (), NonTrivialStruct::Reflect ().mapping);                             \
        NonTrivialStruct nonTrivialStructLoaded;                                                                       \
        CHECK_NOT_EQUAL (nonTrivialStructLoaded, nonTrivialStructChanged);                                             \
        patches[0u].Apply (&nonTrivialStructLoaded);                                                                   \
        CHECK_EQUAL (nonTrivialStructLoaded, nonTrivialStructChanged);                                                 \
                                                                                                                       \
        CHECK_EQUAL (patches[1u].GetTypeMapping (), TrivialStruct::Reflect ().mapping);                                \
        TrivialStruct trivialStructLoaded;                                                                             \
        CHECK_NOT_EQUAL (trivialStructLoaded, trivialStructChanged);                                                   \
        patches[1u].Apply (&trivialStructLoaded);                                                                      \
        CHECK_EQUAL (trivialStructLoaded, trivialStructChanged);                                                       \
                                                                                                                       \
        CHECK_EQUAL (patches[2u].GetTypeMapping (), UnionStruct::Reflect ().mapping);                                  \
        UnionStruct unionStructLoaded;                                                                                 \
        CHECK_NOT_EQUAL (unionStructLoaded, unionStructChanged);                                                       \
        patches[2u].Apply (&unionStructLoaded);                                                                        \
        CHECK_EQUAL (unionStructLoaded, unionStructChanged);                                                           \
    }
