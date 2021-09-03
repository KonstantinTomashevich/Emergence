#include <ostream>
#include <unordered_set>
#include <variant>
#include <vector>

#include <StandardLayout/MappingBuilder.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::StandardLayout::Test
{
bool MappingBuilderTestIncludeMarker () noexcept
{
    return true;
}

struct FieldSeedBase
{
    std::string name;
    std::size_t offset = 0u;
    FieldId recordedId = 0u;
};

struct BitFieldSeed final : public FieldSeedBase
{
    std::uint_fast8_t bitOffset;
};

struct Int8FieldSeed final : public FieldSeedBase
{
};

struct Int16FieldSeed final : public FieldSeedBase
{
};

struct Int32FieldSeed final : public FieldSeedBase
{
};

struct Int64FieldSeed final : public FieldSeedBase
{
};

struct UInt8FieldSeed final : public FieldSeedBase
{
};

struct UInt16FieldSeed final : public FieldSeedBase
{
};

struct UInt32FieldSeed final : public FieldSeedBase
{
};

struct UInt64FieldSeed final : public FieldSeedBase
{
};

struct FloatFieldSeed final : public FieldSeedBase
{
};

struct DoubleFieldSeed final : public FieldSeedBase
{
};

struct StringFieldSeed final : public FieldSeedBase
{
    std::size_t maxSize;
};

struct BlockFieldSeed final : public FieldSeedBase
{
    std::size_t size;
};

struct NestedObjectFieldSeed final : public FieldSeedBase
{
    Mapping typeMapping;
};

using FieldSeed = std::variant<BitFieldSeed,
                               Int8FieldSeed,
                               Int16FieldSeed,
                               Int32FieldSeed,
                               Int64FieldSeed,
                               UInt8FieldSeed,
                               UInt16FieldSeed,
                               UInt32FieldSeed,
                               UInt64FieldSeed,
                               FloatFieldSeed,
                               DoubleFieldSeed,
                               StringFieldSeed,
                               BlockFieldSeed,
                               NestedObjectFieldSeed>;

struct MappingSeed final
{
    std::string name;
    std::size_t objectSize;
    std::vector<FieldSeed> fields;
};

Mapping Grow (MappingSeed &_seed, MappingBuilder &_builder)
{
    _builder.Begin (_seed.name.c_str (), _seed.objectSize);
    for (FieldSeed &packedSeed : _seed.fields)
    {
        std::visit (
            [&_builder] (auto &seed)
            {
                using Seed = std::decay_t<decltype (seed)>;

                if constexpr (std::is_same_v<Seed, BitFieldSeed>)
                {
                    seed.recordedId = _builder.RegisterBit (seed.name.c_str (), seed.offset, seed.bitOffset);
                }
                else if constexpr (std::is_same_v<Seed, Int8FieldSeed>)
                {
                    seed.recordedId = _builder.RegisterInt8 (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, Int16FieldSeed>)
                {
                    seed.recordedId = _builder.RegisterInt16 (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, Int32FieldSeed>)
                {
                    seed.recordedId = _builder.RegisterInt32 (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, Int64FieldSeed>)
                {
                    seed.recordedId = _builder.RegisterInt64 (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, UInt8FieldSeed>)
                {
                    seed.recordedId = _builder.RegisterUInt8 (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, UInt16FieldSeed>)
                {
                    seed.recordedId = _builder.RegisterUInt16 (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, UInt32FieldSeed>)
                {
                    seed.recordedId = _builder.RegisterUInt32 (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, UInt64FieldSeed>)
                {
                    seed.recordedId = _builder.RegisterUInt64 (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, FloatFieldSeed>)
                {
                    seed.recordedId = _builder.RegisterFloat (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, DoubleFieldSeed>)
                {
                    seed.recordedId = _builder.RegisterDouble (seed.name.c_str (), seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, StringFieldSeed>)
                {
                    seed.recordedId = _builder.RegisterString (seed.name.c_str (), seed.offset, seed.maxSize);
                }
                else if constexpr (std::is_same_v<Seed, BlockFieldSeed>)
                {
                    seed.recordedId = _builder.RegisterBlock (seed.name.c_str (), seed.offset, seed.size);
                }
                else if constexpr (std::is_same_v<Seed, NestedObjectFieldSeed>)
                {
                    seed.recordedId = _builder.RegisterNestedObject (seed.name.c_str (), seed.offset, seed.typeMapping);
                }
            },
            packedSeed);
    }

    return _builder.End ();
}

Mapping Grow (MappingSeed _seed)
{
    MappingBuilder builder;
    return Grow (_seed, builder);
}

void GrowAndTest (MappingSeed _seed, MappingBuilder &_builder)
{
    Mapping mapping = Grow (_seed, _builder);
    CHECK_EQUAL (_seed.name, mapping.GetName ());
    std::unordered_set<FieldId> idsFound;

    for (FieldSeed &packedSeed : _seed.fields)
    {
        std::visit (
            [&mapping, &idsFound] (auto &seed)
            {
                using Seed = std::decay_t<decltype (seed)>;
                LOG ("Checking field ", seed.name, "...");
                Field field = mapping.GetField (seed.recordedId);

                CHECK (field.IsHandleValid ());
                CHECK_EQUAL (seed.name, field.GetName ());

                if (field.IsHandleValid ())
                {
                    CHECK_EQUAL (field.GetOffset (), seed.offset);

                    if constexpr (std::is_same_v<Seed, BitFieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), 1u);
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::BIT);
                        CHECK_EQUAL (field.GetBitOffset (), seed.bitOffset);
                    }
                    else if constexpr (std::is_same_v<Seed, Int8FieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (int8_t));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::INT);
                    }
                    else if constexpr (std::is_same_v<Seed, Int16FieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (int16_t));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::INT);
                    }
                    else if constexpr (std::is_same_v<Seed, Int32FieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (int32_t));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::INT);
                    }
                    else if constexpr (std::is_same_v<Seed, Int64FieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (int64_t));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::INT);
                    }
                    else if constexpr (std::is_same_v<Seed, UInt8FieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (uint8_t));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::UINT);
                    }
                    else if constexpr (std::is_same_v<Seed, UInt16FieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (uint16_t));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::UINT);
                    }
                    else if constexpr (std::is_same_v<Seed, UInt32FieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (uint32_t));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::UINT);
                    }
                    else if constexpr (std::is_same_v<Seed, UInt64FieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (uint64_t));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::UINT);
                    }
                    else if constexpr (std::is_same_v<Seed, FloatFieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (float));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::FLOAT);
                    }
                    else if constexpr (std::is_same_v<Seed, DoubleFieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), sizeof (double));
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::FLOAT);
                    }
                    else if constexpr (std::is_same_v<Seed, StringFieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), seed.maxSize);
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::STRING);
                    }
                    else if constexpr (std::is_same_v<Seed, BlockFieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), seed.size);
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::BLOCK);
                    }
                    else if constexpr (std::is_same_v<Seed, NestedObjectFieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), seed.typeMapping.GetObjectSize ());
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::NESTED_OBJECT);
                        CHECK (field.GetNestedObjectMapping () == seed.typeMapping);

                        Mapping::FieldIterator iterator = seed.typeMapping.Begin ();
                        Mapping::FieldIterator end = seed.typeMapping.End ();

                        while (iterator != end)
                        {
                            Field nestedField = *iterator;
                            Field projectedField = mapping.GetField (
                                ProjectNestedField (seed.recordedId, seed.typeMapping.GetFieldId (iterator)));

                            CHECK (projectedField.IsHandleValid ());
                            CHECK_EQUAL (nestedField.GetArchetype (), projectedField.GetArchetype ());
                            CHECK_EQUAL (nestedField.GetSize (), projectedField.GetSize ());
                            CHECK_EQUAL (nestedField.GetOffset () + field.GetOffset (), projectedField.GetOffset ());

                            const std::string expectedProjectedName = seed.name + "." + nestedField.GetName ();
                            CHECK_EQUAL (expectedProjectedName, projectedField.GetName ());

                            switch (nestedField.GetArchetype ())
                            {
                            case FieldArchetype::BIT:
                                CHECK_EQUAL (nestedField.GetBitOffset (), projectedField.GetBitOffset ());

                            case FieldArchetype::INT:
                            case FieldArchetype::UINT:
                            case FieldArchetype::FLOAT:
                            case FieldArchetype::STRING:
                            case FieldArchetype::BLOCK:
                                break;

                            case FieldArchetype::NESTED_OBJECT:
                                CHECK (nestedField.GetNestedObjectMapping () ==
                                       projectedField.GetNestedObjectMapping ());
                            }

                            ++iterator;
                        }
                    }
                }

                CHECK_EQUAL (idsFound.count (seed.recordedId), 0u);
                idsFound.insert (seed.recordedId);

                if constexpr (std::is_same_v<Seed, NestedObjectFieldSeed>)
                {
                    Mapping::FieldIterator iterator = seed.typeMapping.Begin ();
                    Mapping::FieldIterator end = seed.typeMapping.End ();

                    while (iterator != end)
                    {
                        FieldId projectedId =
                            ProjectNestedField (seed.recordedId, seed.typeMapping.GetFieldId (iterator));

                        CHECK_EQUAL (idsFound.count (projectedId), 0u);
                        idsFound.insert (projectedId);
                        ++iterator;
                    }
                }
            },
            packedSeed);
    }

    Mapping::FieldIterator iterator = mapping.Begin ();
    Mapping::FieldIterator end = mapping.End ();

    while (iterator != end)
    {
        FieldId fieldId = mapping.GetFieldId (iterator);
        CHECK_EQUAL (idsFound.count (fieldId), 1u);
        ++iterator;
    }
}

void GrowAndTest (MappingSeed _seed)
{
    MappingBuilder builder;
    GrowAndTest (std::move (_seed), builder);
}

struct TwoIntsTest
{
    uint32_t first;
    int16_t second;
};

static const MappingSeed twoIntsCorrectOrder {"TwoInts",
                                              sizeof (TwoIntsTest),
                                              {
                                                  UInt32FieldSeed {{"first", offsetof (TwoIntsTest, first)}},
                                                  Int16FieldSeed {{"second", offsetof (TwoIntsTest, second)}},
                                              }};

static const MappingSeed twoIntsReversedOrder {"TwoInts",
                                               sizeof (TwoIntsTest),
                                               {
                                                   Int16FieldSeed {{"second", offsetof (TwoIntsTest, second)}},
                                                   UInt32FieldSeed {{"first", offsetof (TwoIntsTest, first)}},
                                               }};

struct AllBasicTypesTest final
{
    uint64_t someFlags;

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

    char block[48u];
    char string[24u];
};

static const MappingSeed allBasicTypes {
    "AllBasicTypes",
    sizeof (AllBasicTypesTest),
    {
        BitFieldSeed {{"flag0", offsetof (AllBasicTypesTest, someFlags)}, 0u},
        BitFieldSeed {{"flag1", offsetof (AllBasicTypesTest, someFlags)}, 1u},
        BitFieldSeed {{"flag2", offsetof (AllBasicTypesTest, someFlags)}, 2u},
        BitFieldSeed {{"flag3", offsetof (AllBasicTypesTest, someFlags)}, 3u},

        Int8FieldSeed {{"int8", offsetof (AllBasicTypesTest, int8)}},
        Int16FieldSeed {{"int16", offsetof (AllBasicTypesTest, int16)}},
        Int32FieldSeed {{"int32", offsetof (AllBasicTypesTest, int32)}},
        Int64FieldSeed {{"int64", offsetof (AllBasicTypesTest, int64)}},

        UInt8FieldSeed {{"uint8", offsetof (AllBasicTypesTest, uint8)}},
        UInt16FieldSeed {{"uint16", offsetof (AllBasicTypesTest, uint16)}},
        UInt32FieldSeed {{"uint32", offsetof (AllBasicTypesTest, uint32)}},
        UInt64FieldSeed {{"uint64", offsetof (AllBasicTypesTest, uint64)}},

        FloatFieldSeed {{"floating", offsetof (AllBasicTypesTest, floating)}},
        DoubleFieldSeed {{"doubleFloating", offsetof (AllBasicTypesTest, doubleFloating)}},

        StringFieldSeed {{"string", offsetof (AllBasicTypesTest, string)}, sizeof (AllBasicTypesTest::string)},
        BlockFieldSeed {{"block", offsetof (AllBasicTypesTest, block)}, sizeof (AllBasicTypesTest::block)},
    }};

struct UnionWithBasicTypesTest
{
    char nonUnionField[24u];
    union
    {
        uint64_t union1UInt64;
        char union1String[16u];
    };

    union
    {
        struct
        {
            int64_t union2Int64;
            float union2Float;
        };

        double union2Double;
    };

    uint64_t nonUnionFlags;
};

static const MappingSeed unionWithBasicTypes {
    "UnionWithBasicTypes",
    sizeof (UnionWithBasicTypesTest),
    {
        BlockFieldSeed {{"nonUnionField", offsetof (UnionWithBasicTypesTest, nonUnionField)},
                        sizeof (UnionWithBasicTypesTest::nonUnionField)},

        UInt64FieldSeed {{"union1UInt64", offsetof (UnionWithBasicTypesTest, union1UInt64)}},
        StringFieldSeed {{"union1String", offsetof (UnionWithBasicTypesTest, union1String)},
                         sizeof (UnionWithBasicTypesTest::union1String)},

        Int64FieldSeed {{"union2Int64", offsetof (UnionWithBasicTypesTest, union2Int64)}},
        FloatFieldSeed {{"union2Float", offsetof (UnionWithBasicTypesTest, union2Float)}},
        DoubleFieldSeed {{"union2Double", offsetof (UnionWithBasicTypesTest, union2Double)}},

        BitFieldSeed {{"nonUnionFlags0", offsetof (UnionWithBasicTypesTest, nonUnionFlags)}, 0u},
        BitFieldSeed {{"nonUnionFlags1", offsetof (UnionWithBasicTypesTest, nonUnionFlags)}, 1u},
        BitFieldSeed {{"nonUnionFlags2", offsetof (UnionWithBasicTypesTest, nonUnionFlags)}, 2u},
        BitFieldSeed {{"nonUnionFlags3", offsetof (UnionWithBasicTypesTest, nonUnionFlags)}, 3u},
    }};

struct NestedOneSublevelTest
{
    uint64_t uint64;
    TwoIntsTest firstNested;
    AllBasicTypesTest secondNested;
    char block[32u];
};

static const MappingSeed nestedOneSublevel {
    "NestedOneSublevel",
    sizeof (NestedOneSublevelTest),
    {
        UInt64FieldSeed {{"uint64", offsetof (NestedOneSublevelTest, uint64)}},
        NestedObjectFieldSeed {{"firstNested", offsetof (NestedOneSublevelTest, firstNested)},
                               Grow (twoIntsCorrectOrder)},
        NestedObjectFieldSeed {{"secondNested", offsetof (NestedOneSublevelTest, secondNested)}, Grow (allBasicTypes)},
        BlockFieldSeed {{"block", offsetof (NestedOneSublevelTest, block)}, sizeof (NestedOneSublevelTest::block)},
    }};

struct NestedInUnionOneSublevelTest
{
    uint64_t uint64;
    union
    {
        TwoIntsTest firstNested;
        AllBasicTypesTest secondNested;
    };

    char block[32u];
};

static MappingSeed nestedInUnionOneSublevel {
    "NestedInUnionOneSublevel",
    sizeof (NestedInUnionOneSublevelTest),
    {
        UInt64FieldSeed {{"uint64", offsetof (NestedInUnionOneSublevelTest, uint64)}},
        NestedObjectFieldSeed {{"firstNested", offsetof (NestedInUnionOneSublevelTest, firstNested)},
                               Grow (twoIntsCorrectOrder)},
        NestedObjectFieldSeed {{"secondNested", offsetof (NestedInUnionOneSublevelTest, secondNested)},
                               Grow (allBasicTypes)},
        BlockFieldSeed {{"block", offsetof (NestedInUnionOneSublevelTest, block)},
                        sizeof (NestedOneSublevelTest::block)},
    }};

struct NestedTwoSublevelsTest
{
    float floating;
    NestedOneSublevelTest firstNested;
    char string[24u];
    NestedInUnionOneSublevelTest secondNested;
};

static MappingSeed nestedTwoSublevels {
    "NestedTwoSublevels",
    sizeof (NestedTwoSublevelsTest),
    {
        FloatFieldSeed {{"floating", offsetof (NestedTwoSublevelsTest, floating)}},
        NestedObjectFieldSeed {{"firstNested", offsetof (NestedTwoSublevelsTest, firstNested)},
                               Grow (nestedOneSublevel)},
        BlockFieldSeed {{"string", offsetof (NestedTwoSublevelsTest, string)}, sizeof (NestedTwoSublevelsTest::string)},
        NestedObjectFieldSeed {{"secondNested", offsetof (NestedTwoSublevelsTest, secondNested)},
                               Grow (nestedInUnionOneSublevel)},
    }};
} // namespace Emergence::StandardLayout::Test

using namespace Emergence::StandardLayout::Test;

BEGIN_SUITE (MappingBuilder)

TEST_CASE (TwoIntsCorrectOrder)
{
    GrowAndTest (twoIntsCorrectOrder);
}

TEST_CASE (TwoIntsReversedOrder)
{
    GrowAndTest (twoIntsReversedOrder);
}

TEST_CASE (AllBasicTypes)
{
    GrowAndTest (allBasicTypes);
}

TEST_CASE (UnionsWithBasicTypes)
{
    GrowAndTest (unionWithBasicTypes);
}

TEST_CASE (NestedOneSublevel)
{
    GrowAndTest (nestedOneSublevel);
}

TEST_CASE (NestedInUnionOneSublevel)
{
    GrowAndTest (nestedInUnionOneSublevel);
}

TEST_CASE (NestedTwoSublevels)
{
    GrowAndTest (nestedTwoSublevels);
}

TEST_CASE (BuildMultipleMappings)
{
    Emergence::StandardLayout::MappingBuilder builder;
    GrowAndTest (twoIntsCorrectOrder, builder);
    GrowAndTest (unionWithBasicTypes, builder);
    GrowAndTest (nestedTwoSublevels, builder);
}

TEST_CASE (BuildMultipleMappingsWhileMovingBuilder)
{
    Emergence::StandardLayout::MappingBuilder firstBuilder;
    GrowAndTest (twoIntsCorrectOrder, firstBuilder);

    Emergence::StandardLayout::MappingBuilder secondBuilder (std::move (firstBuilder));
    GrowAndTest (unionWithBasicTypes, secondBuilder);

    firstBuilder = std::move (secondBuilder);
    GrowAndTest (nestedTwoSublevels, firstBuilder);
}

TEST_CASE (FieldManipulations)
{
    Emergence::StandardLayout::Mapping mapping = Grow (twoIntsCorrectOrder);
    auto iterator = mapping.Begin ();

    REQUIRE (iterator != mapping.End ());
    Emergence::StandardLayout::Field firstField = *iterator;
    ++iterator;

    REQUIRE (iterator != mapping.End ());
    Emergence::StandardLayout::Field secondField = *iterator;

    Emergence::StandardLayout::Field emptyField;

    CHECK (firstField.IsSame (firstField));
    CHECK (!firstField.IsSame (secondField));
    CHECK (!firstField.IsSame (emptyField));

    CHECK (firstField);
    CHECK (!emptyField);

    emptyField = std::move (firstField);
    CHECK (!firstField);
    CHECK (emptyField);

    CHECK (!emptyField.IsSame (secondField));
    emptyField = secondField;
    CHECK (emptyField.IsSame (secondField));
}

TEST_CASE (FieldsIteration)
{
    Emergence::StandardLayout::Mapping mapping = Grow (twoIntsCorrectOrder);
    auto iterator = mapping.Begin ();

    Emergence::StandardLayout::Field firstField = *iterator++;
    Emergence::StandardLayout::Field secondField = *iterator++;

    CHECK (iterator == mapping.End ());
    CHECK (firstField.IsHandleValid ());
    CHECK (secondField.IsHandleValid ());
    CHECK (!firstField.IsSame (secondField));

    CHECK (secondField.IsSame (*--iterator));
    CHECK (firstField.IsSame (*--iterator));

    auto iteratorCopyAtBegin = iterator++;
    CHECK (iteratorCopyAtBegin != iterator);
    CHECK (firstField.IsSame (*iteratorCopyAtBegin));
    CHECK (secondField.IsSame (*iterator));

    auto iteratorCopyAtSecond = iterator--;
    CHECK (iterator == iteratorCopyAtBegin);
    CHECK (iterator != iteratorCopyAtSecond);

    CHECK (firstField.IsSame (*iteratorCopyAtBegin));
    CHECK (firstField.IsSame (*iterator));
    CHECK (secondField.IsSame (*iteratorCopyAtSecond));

    iterator = iteratorCopyAtSecond;
    CHECK (iterator != iteratorCopyAtBegin);
    CHECK (iterator == iteratorCopyAtSecond);

    iterator = std::move (iteratorCopyAtBegin);
    CHECK (iterator == iteratorCopyAtBegin);
    CHECK (iterator != iteratorCopyAtSecond);
}

TEST_CASE (MappingEquality)
{
    Emergence::StandardLayout::Mapping twoIntsCorrectOrderMapping = Grow (twoIntsCorrectOrder);
    Emergence::StandardLayout::Mapping twoIntsCorrectOrderMappingCopy = twoIntsCorrectOrderMapping;
    Emergence::StandardLayout::Mapping twoIntsReversedOrderMapping = Grow (twoIntsReversedOrder);

    CHECK (twoIntsCorrectOrderMapping == twoIntsCorrectOrderMapping);
    CHECK (twoIntsCorrectOrderMappingCopy == twoIntsCorrectOrderMappingCopy);
    CHECK (twoIntsReversedOrderMapping == twoIntsReversedOrderMapping);

    CHECK (twoIntsCorrectOrderMapping == twoIntsCorrectOrderMappingCopy);
    CHECK (twoIntsCorrectOrderMapping != twoIntsReversedOrderMapping);
    CHECK (twoIntsCorrectOrderMappingCopy != twoIntsReversedOrderMapping);

    Emergence::StandardLayout::Mapping twoIntsCorrectOrderMappingMoved = std::move (twoIntsCorrectOrderMapping);
    CHECK (twoIntsCorrectOrderMappingMoved == twoIntsCorrectOrderMappingMoved);
    CHECK (twoIntsCorrectOrderMappingMoved == twoIntsCorrectOrderMappingCopy);
    CHECK (twoIntsCorrectOrderMappingMoved != twoIntsCorrectOrderMapping);
    CHECK (twoIntsCorrectOrderMappingCopy != twoIntsCorrectOrderMapping);
    CHECK (twoIntsCorrectOrderMappingMoved != twoIntsReversedOrderMapping);
}

END_SUITE
