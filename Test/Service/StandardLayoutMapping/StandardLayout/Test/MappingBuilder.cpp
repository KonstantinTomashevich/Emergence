#include <ostream>
#include <unordered_set>
#include <variant>
#include <vector>

#include <StandardLayout/MappingBuilder.hpp>
#include <StandardLayout/Test/MappingBuilder.hpp>

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
            [&_builder] (auto &_seed)
            {
                using Seed = std::decay_t<decltype (_seed)>;

                if constexpr (std::is_same_v<Seed, BitFieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterBit (_seed.name.c_str (), _seed.offset, _seed.bitOffset);
                }
                else if constexpr (std::is_same_v<Seed, Int8FieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterInt8 (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, Int16FieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterInt16 (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, Int32FieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterInt32 (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, Int64FieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterInt64 (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, UInt8FieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterUInt8 (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, UInt16FieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterUInt16 (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, UInt32FieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterUInt32 (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, UInt64FieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterUInt64 (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, FloatFieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterFloat (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, DoubleFieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterDouble (_seed.name.c_str (), _seed.offset);
                }
                else if constexpr (std::is_same_v<Seed, StringFieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterString (_seed.name.c_str (), _seed.offset, _seed.maxSize);
                }
                else if constexpr (std::is_same_v<Seed, BlockFieldSeed>)
                {
                    _seed.recordedId = _builder.RegisterBlock (_seed.name.c_str (), _seed.offset, _seed.size);
                }
                else if constexpr (std::is_same_v<Seed, NestedObjectFieldSeed>)
                {
                    _seed.recordedId =
                        _builder.RegisterNestedObject (_seed.name.c_str (), _seed.offset, _seed.typeMapping);
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
            [&mapping, &idsFound] (auto &_seed)
            {
                using Seed = std::decay_t<decltype (_seed)>;
                LOG ("Checking field ", _seed.name, "...");
                Field field = mapping.GetField (_seed.recordedId);

                CHECK (field.IsHandleValid ());
                CHECK_EQUAL (_seed.name, field.GetName ());

                if (field.IsHandleValid ())
                {
                    CHECK_EQUAL (field.GetOffset (), _seed.offset);

                    if constexpr (std::is_same_v<Seed, BitFieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), 1u);
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::BIT);
                        CHECK_EQUAL (field.GetBitOffset (), _seed.bitOffset);
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
                        CHECK_EQUAL (field.GetSize (), _seed.maxSize);
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::STRING);
                    }
                    else if constexpr (std::is_same_v<Seed, BlockFieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), _seed.size);
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::BLOCK);
                    }
                    else if constexpr (std::is_same_v<Seed, NestedObjectFieldSeed>)
                    {
                        CHECK_EQUAL (field.GetSize (), _seed.typeMapping.GetObjectSize ());
                        CHECK_EQUAL (field.GetArchetype (), FieldArchetype::NESTED_OBJECT);
                        CHECK (field.GetNestedObjectMapping () == _seed.typeMapping);

                        Mapping::FieldIterator iterator = _seed.typeMapping.Begin ();
                        Mapping::FieldIterator end = _seed.typeMapping.End ();

                        while (iterator != end)
                        {
                            Field nestedField = *iterator;
                            Field projectedField = mapping.GetField (
                                ProjectNestedField (_seed.recordedId, _seed.typeMapping.GetFieldId (iterator)));

                            CHECK (projectedField.IsHandleValid ());
                            CHECK_EQUAL (nestedField.GetArchetype (), projectedField.GetArchetype ());
                            CHECK_EQUAL (nestedField.GetSize (), projectedField.GetSize ());
                            CHECK_EQUAL (nestedField.GetOffset () + field.GetOffset (), projectedField.GetOffset ());

                            const std::string expectedProjectedName =
                                _seed.name + PROJECTION_NAME_SEPARATOR + nestedField.GetName ();
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

                CHECK_EQUAL (idsFound.count (_seed.recordedId), 0u);
                idsFound.insert (_seed.recordedId);

                if constexpr (std::is_same_v<Seed, NestedObjectFieldSeed>)
                {
                    Mapping::FieldIterator iterator = _seed.typeMapping.Begin ();
                    Mapping::FieldIterator end = _seed.typeMapping.End ();

                    while (iterator != end)
                    {
                        FieldId projectedId =
                            ProjectNestedField (_seed.recordedId, _seed.typeMapping.GetFieldId (iterator));

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

static const MappingSeed TWO_INTS_CORRECT_ORDER {"TwoInts",
                                                 sizeof (TwoIntsTest),
                                                 {
                                                     UInt32FieldSeed {{"first", offsetof (TwoIntsTest, first)}},
                                                     Int16FieldSeed {{"second", offsetof (TwoIntsTest, second)}},
                                                 }};

static const MappingSeed TWO_INTS_REVERSED_ORDER {"TwoInts",
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

static const MappingSeed ALL_BASIC_TYPES {
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

static const MappingSeed UNION_WITH_BASIC_TYPES {
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

static const MappingSeed NESTED_ONE_SUBLEVEL {
    "NestedOneSublevel",
    sizeof (NestedOneSublevelTest),
    {
        UInt64FieldSeed {{"uint64", offsetof (NestedOneSublevelTest, uint64)}},
        NestedObjectFieldSeed {{"firstNested", offsetof (NestedOneSublevelTest, firstNested)},
                               Grow (TWO_INTS_CORRECT_ORDER)},
        NestedObjectFieldSeed {{"secondNested", offsetof (NestedOneSublevelTest, secondNested)},
                               Grow (ALL_BASIC_TYPES)},
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
                               Grow (TWO_INTS_CORRECT_ORDER)},
        NestedObjectFieldSeed {{"secondNested", offsetof (NestedInUnionOneSublevelTest, secondNested)},
                               Grow (ALL_BASIC_TYPES)},
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
                               Grow (NESTED_ONE_SUBLEVEL)},
        BlockFieldSeed {{"string", offsetof (NestedTwoSublevelsTest, string)}, sizeof (NestedTwoSublevelsTest::string)},
        NestedObjectFieldSeed {{"secondNested", offsetof (NestedTwoSublevelsTest, secondNested)},
                               Grow (nestedInUnionOneSublevel)},
    }};
} // namespace Emergence::StandardLayout::Test

using namespace Emergence::StandardLayout::Test;

BEGIN_SUITE (MappingBuilder)

TEST_CASE (TwoIntsCorrectOrder)
{
    GrowAndTest (TWO_INTS_CORRECT_ORDER);
}

TEST_CASE (TwoIntsReversedOrder)
{
    GrowAndTest (TWO_INTS_REVERSED_ORDER);
}

TEST_CASE (AllBasicTypes)
{
    GrowAndTest (ALL_BASIC_TYPES);
}

TEST_CASE (UnionsWithBasicTypes)
{
    GrowAndTest (UNION_WITH_BASIC_TYPES);
}

TEST_CASE (NestedOneSublevel)
{
    GrowAndTest (NESTED_ONE_SUBLEVEL);
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
    GrowAndTest (TWO_INTS_CORRECT_ORDER, builder);
    GrowAndTest (UNION_WITH_BASIC_TYPES, builder);
    GrowAndTest (nestedTwoSublevels, builder);
}

TEST_CASE (BuildMultipleMappingsWhileMovingBuilder)
{
    Emergence::StandardLayout::MappingBuilder firstBuilder;
    GrowAndTest (TWO_INTS_CORRECT_ORDER, firstBuilder);

    Emergence::StandardLayout::MappingBuilder secondBuilder (std::move (firstBuilder));
    GrowAndTest (UNION_WITH_BASIC_TYPES, secondBuilder);

    firstBuilder = std::move (secondBuilder);
    GrowAndTest (nestedTwoSublevels, firstBuilder);
}

TEST_CASE (FieldManipulations)
{
    Emergence::StandardLayout::Mapping mapping = Grow (TWO_INTS_CORRECT_ORDER);
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
    // NOLINTNEXTLINE(bugprone-use-after-move): Moved-out fields will have invalid handle.
    CHECK (!firstField);
    CHECK (emptyField);

    CHECK (!emptyField.IsSame (secondField));
    emptyField = secondField;
    CHECK (emptyField.IsSame (secondField));
}

TEST_CASE (FieldsIteration)
{
    Emergence::StandardLayout::Mapping mapping = Grow (TWO_INTS_CORRECT_ORDER);
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
    Emergence::StandardLayout::Mapping twoIntsCorrectOrderMapping = Grow (TWO_INTS_CORRECT_ORDER);
    Emergence::StandardLayout::Mapping twoIntsCorrectOrderMappingCopy = twoIntsCorrectOrderMapping;
    Emergence::StandardLayout::Mapping twoIntsReversedOrderMapping = Grow (TWO_INTS_REVERSED_ORDER);

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
