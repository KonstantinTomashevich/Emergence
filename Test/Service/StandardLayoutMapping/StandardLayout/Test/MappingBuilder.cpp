#include <ostream>
#include <unordered_set>
#include <utility>
#include <variant>

#include <StandardLayout/MappingBuilder.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::StandardLayout::Test
{
bool MappingBuilderTestIncludeMarker () noexcept
{
    return true;
}

static void CheckMappingEquality (const Mapping &_first, const Mapping &_second) noexcept;

static void CheckFieldEquality (const Field &_first, const Field &_second, bool _requireOffsetEquality = true) noexcept
{
    CHECK (_first.IsHandleValid ());
    CHECK (_second.IsHandleValid ());
    CHECK_EQUAL (_first.GetArchetype (), _second.GetArchetype ());

    if (_requireOffsetEquality)
    {
        CHECK_EQUAL (_first.GetOffset (), _second.GetOffset ());
    }

    CHECK_EQUAL (_first.GetSize (), _second.GetSize ());
    switch (_first.GetArchetype ())
    {
        case FieldArchetype::BIT:
            CHECK_EQUAL (_first.GetBitOffset (), _second.GetBitOffset ());
            return;

        case FieldArchetype::INT:
        case FieldArchetype::UINT:
        case FieldArchetype::FLOAT:
        case FieldArchetype::STRING:
        case FieldArchetype::BLOCK:
            return;

        case FieldArchetype::NESTED_OBJECT:
            CheckMappingEquality (_first.GetNestedObjectMapping (), _second.GetNestedObjectMapping ());
            return;
    }

    CHECK_WITH_MESSAGE (
        false, "Found unknown archetype value ",
        static_cast <uint64_t> (_first.GetArchetype ()), ". Possible memory corruption.");
}

static void CheckMappingEquality (const Mapping &_first, const Mapping &_second) noexcept
{
    // There is no better implementation agnostic way of checking mapping equality than to check equality of all fields.
    // Also, we will assume that if mappings are truly equal, than order of fields in them is equal too.

    Mapping::FieldIterator firstIterator = _first.Begin ();
    Mapping::FieldIterator firstEnd = _first.End ();

    Mapping::FieldIterator secondIterator = _second.Begin ();
    Mapping::FieldIterator secondEnd = _second.End ();

    while (firstIterator != firstEnd)
    {
        if (secondIterator == secondEnd)
        {
            CHECK_WITH_MESSAGE (false, "Second mapping has less fields than first!");
            return;
        }

        LOG ("Checking fields with ids ", static_cast <uint64_t> (_first.GetFieldId (firstIterator)),
             " and ", static_cast <uint64_t> (_second.GetFieldId (secondIterator)), ".");

        CheckFieldEquality (*firstIterator, *secondIterator);
        ++firstIterator;
        ++secondIterator;
    }

    CHECK_WITH_MESSAGE (secondIterator == secondEnd, "Second mapping has more fields than first!");
}

struct FieldSeedBase
{
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

using FieldSeed = std::variant <
    BitFieldSeed,
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

class MappingSeed final
{
public:
    MappingSeed (std::size_t _objectSize, std::vector <FieldSeed> _fields)
        : fields (std::move (_fields)),
          objectSize (_objectSize)
    {
    }

    Mapping Grow ()
    {
        MappingBuilder builder;
        return Grow (builder);
    }

    Mapping Grow (MappingBuilder &_builder)
    {
        _builder.Begin (objectSize);
        for (FieldSeed &seed : fields)
        {
            std::visit (
                [&_builder] (auto &unwrappedSeed)
                {
                    using Seed = std::decay_t <decltype (unwrappedSeed)>;

                    if constexpr (std::is_same_v <Seed, BitFieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterBit (
                            unwrappedSeed.offset, static_cast <BitFieldSeed &> (unwrappedSeed).bitOffset);
                    }
                    else if constexpr (std::is_same_v <Seed, Int8FieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterInt8 (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, Int16FieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterInt16 (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, Int32FieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterInt32 (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, Int64FieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterInt64 (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, UInt8FieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterUInt8 (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, UInt16FieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterUInt16 (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, UInt32FieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterUInt32 (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, UInt64FieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterUInt64 (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, FloatFieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterFloat (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, DoubleFieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterDouble (unwrappedSeed.offset);
                    }
                    else if constexpr (std::is_same_v <Seed, StringFieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterString (
                            unwrappedSeed.offset, static_cast <StringFieldSeed &> (unwrappedSeed).maxSize);
                    }
                    else if constexpr (std::is_same_v <Seed, BlockFieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterBlock (
                            unwrappedSeed.offset, static_cast <BlockFieldSeed &> (unwrappedSeed).size);
                    }
                    else if constexpr (std::is_same_v <Seed, NestedObjectFieldSeed>)
                    {
                        unwrappedSeed.recordedId = _builder.RegisterNestedObject (
                            unwrappedSeed.offset,
                            static_cast <NestedObjectFieldSeed &> (unwrappedSeed).typeMapping);
                    }
                },
                seed);
        }

        return _builder.End ();
    }

    void GrowAndTest ()
    {
        MappingBuilder builder;
        GrowAndTest (builder);
    }

    void GrowAndTest (MappingBuilder &_builder)
    {
        Mapping mapping = Grow (_builder);
        std::unordered_set < FieldId > idsFound;

        for (FieldSeed &seed : fields)
        {
            std::visit (
                [&mapping, &idsFound] (auto &unwrappedSeed)
                {
                    using Seed = std::decay_t <decltype (unwrappedSeed)>;
                    LOG ("Checking field ", unwrappedSeed.recordedId, "...");
                    Field field = mapping.GetField (unwrappedSeed.recordedId);
                    CHECK (field.IsHandleValid ());

                    if (field.IsHandleValid ())
                    {
                        CHECK_EQUAL (field.GetOffset (), unwrappedSeed.offset);

                        if constexpr (std::is_same_v <Seed, BitFieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), 1u);
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::BIT);

                            CHECK_EQUAL (
                                field.GetBitOffset (), static_cast <BitFieldSeed &> (unwrappedSeed).bitOffset);
                        }
                        else if constexpr (std::is_same_v <Seed, Int8FieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (int8_t));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::INT);
                        }
                        else if constexpr (std::is_same_v <Seed, Int16FieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (int16_t));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::INT);
                        }
                        else if constexpr (std::is_same_v <Seed, Int32FieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (int32_t));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::INT);
                        }
                        else if constexpr (std::is_same_v <Seed, Int64FieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (int64_t));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::INT);
                        }
                        else if constexpr (std::is_same_v <Seed, UInt8FieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (uint8_t));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::UINT);
                        }
                        else if constexpr (std::is_same_v <Seed, UInt16FieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (uint16_t));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::UINT);
                        }
                        else if constexpr (std::is_same_v <Seed, UInt32FieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (uint32_t));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::UINT);
                        }
                        else if constexpr (std::is_same_v <Seed, UInt64FieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (uint64_t));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::UINT);
                        }
                        else if constexpr (std::is_same_v <Seed, FloatFieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (float));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::FLOAT);
                        }
                        else if constexpr (std::is_same_v <Seed, DoubleFieldSeed>)
                        {
                            CHECK_EQUAL (field.GetSize (), sizeof (double));
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::FLOAT);
                        }
                        else if constexpr (std::is_same_v <Seed, StringFieldSeed>)
                        {
                            CHECK_EQUAL (
                                field.GetSize (), static_cast <StringFieldSeed &> (unwrappedSeed).maxSize);
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::STRING);
                        }
                        else if constexpr (std::is_same_v <Seed, BlockFieldSeed>)
                        {
                            CHECK_EQUAL (
                                field.GetSize (), static_cast <BlockFieldSeed &> (unwrappedSeed).size);
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::BLOCK);
                        }
                        else if constexpr (std::is_same_v <Seed, NestedObjectFieldSeed>)
                        {
                            auto &castedSeed = static_cast <NestedObjectFieldSeed &> (unwrappedSeed);
                            CHECK_EQUAL (field.GetSize (), castedSeed.typeMapping.GetObjectSize ());
                            CHECK_EQUAL (field.GetArchetype (), FieldArchetype::NESTED_OBJECT);
                            CheckMappingEquality (field.GetNestedObjectMapping (), castedSeed.typeMapping);

                            Mapping::FieldIterator iterator = castedSeed.typeMapping.Begin ();
                            Mapping::FieldIterator end = castedSeed.typeMapping.End ();

                            while (iterator != end)
                            {
                                Field nestedField = *iterator;
                                Field projectedField = mapping.GetField (
                                    ProjectNestedField (castedSeed.recordedId,
                                                        castedSeed.typeMapping.GetFieldId (iterator)));

                                CHECK (projectedField.IsHandleValid ());
                                CheckFieldEquality (nestedField, projectedField, false);

                                CHECK_EQUAL (nestedField.GetOffset () + field.GetOffset (),
                                             projectedField.GetOffset ());
                                ++iterator;
                            }
                        }
                    }

                    CHECK_EQUAL (idsFound.count (unwrappedSeed.recordedId), 0u);
                    idsFound.insert (unwrappedSeed.recordedId);

                    if constexpr (std::is_same_v <Seed, NestedObjectFieldSeed>)
                    {
                        auto &castedSeed = static_cast <NestedObjectFieldSeed &> (unwrappedSeed);
                        Mapping::FieldIterator iterator = castedSeed.typeMapping.Begin ();
                        Mapping::FieldIterator end = castedSeed.typeMapping.End ();

                        while (iterator != end)
                        {
                            FieldId projectedId = ProjectNestedField (
                                castedSeed.recordedId, castedSeed.typeMapping.GetFieldId (iterator));

                            CHECK_EQUAL (idsFound.count (projectedId), 0u);
                            idsFound.insert (projectedId);
                            ++iterator;
                        }
                    }
                },
                seed);
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

private:
    std::vector <FieldSeed> fields;
    std::size_t objectSize;
};

struct TwoIntsTest
{
    uint32_t first;
    int16_t second;
};

static MappingSeed twoIntsCorrectOrder (
    sizeof (TwoIntsTest),
    {
        UInt32FieldSeed {{offsetof (TwoIntsTest, first)}},
        Int16FieldSeed {{offsetof(TwoIntsTest, second)}},
    });

static MappingSeed twoIntsReversedOrder (
    sizeof (TwoIntsTest),
    {
        Int16FieldSeed {{offsetof (TwoIntsTest, second)}},
        UInt32FieldSeed {{offsetof (TwoIntsTest, first)}},
    });

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

    char block[48u];
    char string[24u];
};

static MappingSeed allBasicTypes (
    sizeof (AllBasicTypesTest),
    {
        BitFieldSeed {{offsetof (AllBasicTypesTest, someFlags)}, 0u},
        BitFieldSeed {{offsetof (AllBasicTypesTest, someFlags)}, 1u},
        BitFieldSeed {{offsetof (AllBasicTypesTest, someFlags)}, 2u},
        BitFieldSeed {{offsetof (AllBasicTypesTest, someFlags)}, 3u},

        Int8FieldSeed {{offsetof (AllBasicTypesTest, int8)}},
        Int16FieldSeed {{offsetof (AllBasicTypesTest, int16)}},
        Int32FieldSeed {{offsetof (AllBasicTypesTest, int32)}},
        Int64FieldSeed {{offsetof (AllBasicTypesTest, int64)}},

        UInt8FieldSeed {{offsetof (AllBasicTypesTest, uint8)}},
        UInt16FieldSeed {{offsetof (AllBasicTypesTest, uint16)}},
        UInt32FieldSeed {{offsetof (AllBasicTypesTest, uint32)}},
        UInt64FieldSeed {{offsetof (AllBasicTypesTest, uint64)}},

        StringFieldSeed {{offsetof (AllBasicTypesTest, string)}, sizeof (AllBasicTypesTest::string)},
        BlockFieldSeed {{offsetof (AllBasicTypesTest, block)}, sizeof (AllBasicTypesTest::block)},
    });

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

static MappingSeed unionWithBasicTypes (
    sizeof (UnionWithBasicTypesTest),
    {
        BlockFieldSeed {{offsetof (UnionWithBasicTypesTest, nonUnionField)},
                        sizeof (UnionWithBasicTypesTest::nonUnionField)},

        UInt64FieldSeed {{offsetof (UnionWithBasicTypesTest, union1UInt64)}},
        StringFieldSeed {{offsetof (UnionWithBasicTypesTest, union1String)},
                         sizeof (UnionWithBasicTypesTest::union1String)},

        Int64FieldSeed {{offsetof (UnionWithBasicTypesTest, union2Int64)}},
        FloatFieldSeed {{offsetof (UnionWithBasicTypesTest, union2Float)}},
        DoubleFieldSeed {{offsetof (UnionWithBasicTypesTest, union2Double)}},

        BitFieldSeed {{offsetof (UnionWithBasicTypesTest, nonUnionFlags)}, 0u},
        BitFieldSeed {{offsetof (UnionWithBasicTypesTest, nonUnionFlags)}, 1u},
        BitFieldSeed {{offsetof (UnionWithBasicTypesTest, nonUnionFlags)}, 2u},
        BitFieldSeed {{offsetof (UnionWithBasicTypesTest, nonUnionFlags)}, 3u},
    });

struct NestedOneSublevelTest
{
    uint64_t uint64;
    TwoIntsTest firstNested;
    TwoIntsTest secondNested;
    char block[32u];
};

static MappingSeed nestedOneSublevel (
    sizeof (NestedOneSublevelTest),
    {
        UInt64FieldSeed {{offsetof (NestedOneSublevelTest, uint64)}},
        NestedObjectFieldSeed {{offsetof (NestedOneSublevelTest, firstNested)}, twoIntsCorrectOrder.Grow ()},
        NestedObjectFieldSeed {{offsetof (NestedOneSublevelTest, secondNested)}, twoIntsReversedOrder.Grow ()},
        BlockFieldSeed {{offsetof (NestedOneSublevelTest, block)}, sizeof (NestedOneSublevelTest::block)},
    });

struct NestedInUnionOneSublevelTest
{
    uint64_t uint64;
    union
    {
        TwoIntsTest firstNested;
        TwoIntsTest secondNested;
    };

    char block[32u];
};

static MappingSeed nestedInUnionOneSublevel (
    sizeof (NestedInUnionOneSublevelTest),
    {
        UInt64FieldSeed {{offsetof (NestedInUnionOneSublevelTest, uint64)}},
        NestedObjectFieldSeed {{offsetof (NestedInUnionOneSublevelTest, firstNested)}, twoIntsCorrectOrder.Grow ()},
        NestedObjectFieldSeed {{offsetof (NestedInUnionOneSublevelTest, secondNested)},
                               twoIntsReversedOrder.Grow ()},
        BlockFieldSeed {{offsetof (NestedInUnionOneSublevelTest, block)}, sizeof (NestedOneSublevelTest::block)},
    });

struct NestedTwoSublevelsTest
{
    float floatField;
    NestedOneSublevelTest firstNested;
    char string[24u];
    NestedInUnionOneSublevelTest secondNested;
};

static MappingSeed nestedTwoSublevels (
    sizeof (NestedTwoSublevelsTest),
    {
        FloatFieldSeed {{offsetof (NestedTwoSublevelsTest, floatField)}},
        NestedObjectFieldSeed {{offsetof (NestedTwoSublevelsTest, firstNested)}, nestedOneSublevel.Grow ()},
        BlockFieldSeed {{offsetof (NestedTwoSublevelsTest, string)}, sizeof (NestedTwoSublevelsTest::string)},
        NestedObjectFieldSeed {{offsetof (NestedTwoSublevelsTest, secondNested)}, nestedInUnionOneSublevel.Grow ()},
    });
} // namespace Emergence::StandardLayout::Test

BEGIN_SUITE (MappingBuilder)

TEST_CASE (TwoIntsCorrectOrder)
{
    Emergence::StandardLayout::Test::twoIntsCorrectOrder.GrowAndTest ();
}

TEST_CASE (TwoIntsReversedOrder)
{
    Emergence::StandardLayout::Test::twoIntsReversedOrder.GrowAndTest ();
}

TEST_CASE (AllBasicTypes)
{
    Emergence::StandardLayout::Test::allBasicTypes.GrowAndTest ();
}

TEST_CASE (UnionsWithBasicTypes)
{
    Emergence::StandardLayout::Test::unionWithBasicTypes.GrowAndTest ();
}

TEST_CASE (NestedOneSublevel)
{
    Emergence::StandardLayout::Test::nestedOneSublevel.GrowAndTest ();
}

TEST_CASE (NestedInUnionOneSublevel)
{
    Emergence::StandardLayout::Test::nestedInUnionOneSublevel.GrowAndTest ();
}

TEST_CASE (NestedTwoSublevels)
{
    Emergence::StandardLayout::Test::nestedTwoSublevels.GrowAndTest ();
}

TEST_CASE (BuildMultipleMappings)
{
    Emergence::StandardLayout::MappingBuilder builder;
    Emergence::StandardLayout::Test::twoIntsCorrectOrder.GrowAndTest (builder);
    Emergence::StandardLayout::Test::unionWithBasicTypes.GrowAndTest (builder);
    Emergence::StandardLayout::Test::nestedTwoSublevels.GrowAndTest (builder);
}

END_SUITE