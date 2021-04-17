#include <ostream>
#include <unordered_set>
#include <utility>
#include <variant>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <StandardLayout/MappingBuilder.hpp>

// TODO: Clean up, use less virtuals?
namespace Emergence::StandardLayout::Test
{
bool MappingBuilderTestIncludeMarker () noexcept
{
    return true;
}

static bool operator == (const Mapping &_first, const Mapping &_second) noexcept;

static bool operator == (const Field &_first, const Field &_second) noexcept
{
    if (!_first.IsHandleValid () || !_second.IsHandleValid ())
    {
        return false;
    }

    if (_first.GetArchetype () != _second.GetArchetype () ||
        _first.GetOffset () != _second.GetOffset () ||
        _first.GetSize () != _second.GetSize ())
    {
        return false;
    }

    switch (_first.GetArchetype ())
    {
        case FieldArchetype::BIT:
            return _first.GetBitOffset () == _second.GetBitOffset ();

        case FieldArchetype::INT:
        case FieldArchetype::UINT:
        case FieldArchetype::FLOAT:
        case FieldArchetype::STRING:
        case FieldArchetype::BLOCK:
            return true;

        case FieldArchetype::INSTANCE:
            return _first.GetInstanceMapping () == _second.GetInstanceMapping ();
    }

    BOOST_CHECK_MESSAGE (false, "Found unknown archetype value. Possible memory corruption");
    return false;
}

static bool operator != (const Field &_first, const Field &_second) noexcept
{
    return !(_first == _second);
}

static bool operator == (const Mapping &_first, const Mapping &_second) noexcept
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
            return false;
        }

        if (*firstIterator != *secondIterator)
        {
            return false;
        }

        ++firstIterator;
        ++secondIterator;
    }

    return secondIterator == secondEnd;
}

class FieldSeedBase
{
public:
    void Check (const Mapping &_mapping) const
    {
        BOOST_TEST_MESSAGE (boost::format ("Checking field %1%...") % recordedId);
        Field field = _mapping.GetField (recordedId);
        BOOST_CHECK (field.IsHandleValid ());

        if (field.IsHandleValid ())
        {
            BOOST_CHECK_EQUAL (field.GetOffset (), offset);
            BOOST_CHECK_EQUAL (field.GetSize (), GetSize ());
            ContinueCheck (_mapping, field);
        }
    }

    virtual std::size_t GetSize () const = 0;

    size_t GetOffset () const
    {
        return offset;
    }

    virtual void ExtractIds (std::unordered_set <FieldId> &into) const
    {
        BOOST_CHECK_EQUAL (into.count (recordedId), 0u);
        into.insert (recordedId);
    }

protected:
    explicit FieldSeedBase (size_t _offset)
        : offset (_offset)
    {
    }

    virtual void ContinueCheck (const Mapping &_mapping, Field _field) const = 0;

    FieldId recordedId = 0u;
    std::size_t offset;
};

class BitFieldSeed final : public FieldSeedBase
{
public:
    BitFieldSeed (size_t _offset, uint_fast8_t _bitOffset)
        : FieldSeedBase (_offset),
          bitOffset (_bitOffset)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterBit (offset, bitOffset);
    }

    size_t GetSize () const override
    {
        return 1u;
    }

protected:
    void ContinueCheck (const Mapping &_mapping, Field _field) const override
    {
        BOOST_CHECK_EQUAL (_field.GetArchetype (), FieldArchetype::BIT);
        BOOST_CHECK_EQUAL (_field.GetBitOffset (), bitOffset);
    }

private:
    std::uint_fast8_t bitOffset;
};

template <typename IntType>
class IntFieldSeedBase : public FieldSeedBase
{
public:
    explicit IntFieldSeedBase (size_t _offset)
        : FieldSeedBase (_offset)
    {
    }

    std::size_t GetSize () const override
    {
        return sizeof (IntType);
    }

protected:
    void ContinueCheck (const Mapping &_mapping, Field _field) const override
    {
        BOOST_CHECK_EQUAL (_field.GetArchetype (), FieldArchetype::INT);
    }
};

class Int8FieldSeed : public IntFieldSeedBase <int8_t>
{
public:
    explicit Int8FieldSeed (size_t _offset)
        : IntFieldSeedBase (_offset)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterInt8 (offset);
    }
};

class Int16FieldSeed : public IntFieldSeedBase <int16_t>
{
public:
    explicit Int16FieldSeed (size_t _offset)
        : IntFieldSeedBase (_offset)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterInt16 (offset);
    }
};

class Int32FieldSeed : public IntFieldSeedBase <int32_t>
{
public:
    explicit Int32FieldSeed (size_t _offset)
        : IntFieldSeedBase (_offset)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterInt32 (offset);
    }
};

class Int64FieldSeed : public IntFieldSeedBase <int64_t>
{
public:
    explicit Int64FieldSeed (size_t _offset)
        : IntFieldSeedBase (_offset)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterInt64 (offset);
    }
};

template <typename UIntType>
class UIntFieldSeedBase : public FieldSeedBase
{
public:
    explicit UIntFieldSeedBase (size_t _offset)
        : FieldSeedBase (_offset)
    {
    }

    size_t GetSize () const override
    {
        return sizeof (UIntType);
    }

protected:
    void ContinueCheck (const Mapping &_mapping, Field _field) const override
    {
        BOOST_CHECK_EQUAL (_field.GetArchetype (), FieldArchetype::UINT);
    }
};

class UInt8FieldSeed : public UIntFieldSeedBase <uint8_t>
{
public:
    explicit UInt8FieldSeed (size_t _offset)
        : UIntFieldSeedBase (_offset)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterUInt8 (offset);
    }
};

class UInt16FieldSeed : public UIntFieldSeedBase <uint16_t>
{
public:
    explicit UInt16FieldSeed (size_t _offset)
        : UIntFieldSeedBase (_offset)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterUInt16 (offset);
    }
};

class UInt32FieldSeed : public UIntFieldSeedBase <uint32_t>
{
public:
    explicit UInt32FieldSeed (size_t _offset)
        : UIntFieldSeedBase (_offset)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterUInt32 (offset);
    }
};

class UInt64FieldSeed : public UIntFieldSeedBase <uint64_t>
{
public:
    explicit UInt64FieldSeed (size_t _offset)
        : UIntFieldSeedBase (_offset)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterUInt64 (offset);
    }
};

class StringFieldSeed final : public FieldSeedBase
{
public:
    StringFieldSeed (size_t _offset, size_t _maxSize)
        : FieldSeedBase (_offset),
          maxSize (_maxSize)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterString (offset, maxSize);
    }

    size_t GetSize () const override
    {
        return maxSize;
    }

protected:
    void ContinueCheck (const Mapping &_mapping, Field _field) const override
    {
        BOOST_CHECK_EQUAL (_field.GetArchetype (), FieldArchetype::STRING);
    }

private:
    std::size_t maxSize;
};

class BlockFieldSeed final : public FieldSeedBase
{
public:
    BlockFieldSeed (size_t _offset, size_t _size)
        : FieldSeedBase (_offset),
          size (_size)
    {
    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterBlock (offset, size);
    }

    size_t GetSize () const override
    {
        return size;
    }

protected:
    void ContinueCheck (const Mapping &_mapping, Field _field) const override
    {
        BOOST_CHECK_EQUAL (_field.GetArchetype (), FieldArchetype::BLOCK);
    }

private:
    std::size_t size;
};

class NestedObjectFieldSeed final : public FieldSeedBase
{
public:
    NestedObjectFieldSeed (size_t _offset, Mapping _typeMapping)
        : FieldSeedBase (_offset),
          typeMapping (std::move (_typeMapping))
    {

    }

    void Register (MappingBuilder &_builder)
    {
        recordedId = _builder.RegisterNestedObject (offset, typeMapping);
    }

    size_t GetSize () const override
    {
        return typeMapping.GetObjectSize ();
    }

    void ExtractIds (std::unordered_set <FieldId> &into) const override
    {
        FieldSeedBase::ExtractIds (into);
        Mapping::FieldIterator iterator = typeMapping.Begin ();
        Mapping::FieldIterator end = typeMapping.End ();

        while (iterator != end)
        {
            FieldId projectedId = ProjectNestedField (recordedId, typeMapping.GetFieldId (iterator));
            BOOST_CHECK_EQUAL (into.count (projectedId), 0u);
            into.insert (projectedId);
            ++iterator;
        }
    }

protected:
    void ContinueCheck (const Mapping &_mapping, Field _field) const override
    {
        BOOST_CHECK_EQUAL (_field.GetArchetype (), FieldArchetype::INSTANCE);
        BOOST_CHECK_EQUAL (_field.GetInstanceMapping (), typeMapping);

        Mapping::FieldIterator iterator = typeMapping.Begin ();
        Mapping::FieldIterator end = typeMapping.End ();

        while (iterator != end)
        {
            Field nestedField = *iterator;
            Field projectedField =
                _mapping.GetField (ProjectNestedField (recordedId, typeMapping.GetFieldId (iterator)));

            BOOST_CHECK (projectedField.IsHandleValid ());
            BOOST_CHECK_EQUAL (nestedField, projectedField);
            ++iterator;
        }
    }

private:
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
    StringFieldSeed,
    BlockFieldSeed,
    NestedObjectFieldSeed>;

class MappingSeed final
{
public:
    MappingSeed (std::vector <FieldSeed> _fields)
        : fields (std::move (_fields))
    {
        objectSize = 0u;
        for (const FieldSeed &seed : fields)
        {
            std::visit (
                [this] (const auto &unwrappedSeed)
                {
                    objectSize = std::max (objectSize, unwrappedSeed.GetOffset () + unwrappedSeed.GetSize ());
                },
                seed);
        }
    }

    Mapping Grow (MappingBuilder &_builder)
    {
        _builder.Begin (objectSize);
        for (FieldSeed &seed : fields)
        {
            std::visit (
                [&_builder] (auto &unwrappedSeed)
                {
                    unwrappedSeed.Register (_builder);
                },
                seed);
        }

        return _builder.End ();
    }

    void GrowAndTest (MappingBuilder &_builder)
    {
        Mapping mapping = Grow (_builder);
        std::unordered_set <FieldId> idsFound;

        for (FieldSeed &seed : fields)
        {
            std::visit (
                [&mapping, &idsFound] (auto &unwrappedSeed)
                {
                    unwrappedSeed.Check (mapping);
                    unwrappedSeed.ExtractIds (idsFound);
                },
                seed);
        }

        Mapping::FieldIterator iterator = mapping.Begin ();
        Mapping::FieldIterator end = mapping.End ();

        while (iterator != end)
        {
            FieldId fieldId = mapping.GetFieldId (iterator);
            BOOST_CHECK_EQUAL (idsFound.count (fieldId), 1u);
            ++iterator;
        }
    }

private:
    std::vector <FieldSeed> fields;
    std::size_t objectSize;
};

struct AllBasicTypesStruct final
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
    {
        BitFieldSeed (offsetof(AllBasicTypesStruct, someFlags), 0u),
        BitFieldSeed (offsetof(AllBasicTypesStruct, someFlags), 1u),
        BitFieldSeed (offsetof(AllBasicTypesStruct, someFlags), 2u),
        BitFieldSeed (offsetof(AllBasicTypesStruct, someFlags), 3u),
        Int8FieldSeed (offsetof(AllBasicTypesStruct, int8)),
        Int16FieldSeed (offsetof(AllBasicTypesStruct, int16)),
        Int32FieldSeed (offsetof(AllBasicTypesStruct, int32)),
        Int64FieldSeed (offsetof(AllBasicTypesStruct, int64)),
        UInt8FieldSeed (offsetof(AllBasicTypesStruct, uint8)),
        UInt16FieldSeed (offsetof(AllBasicTypesStruct, uint16)),
        UInt32FieldSeed (offsetof(AllBasicTypesStruct, uint32)),
        UInt64FieldSeed (offsetof(AllBasicTypesStruct, uint64)),
        StringFieldSeed (offsetof(AllBasicTypesStruct, string), sizeof (AllBasicTypesStruct::string)),
        BlockFieldSeed (offsetof(AllBasicTypesStruct, block), sizeof (AllBasicTypesStruct::block)),
    });
} // namespace Emergence::StandardLayout::Test

namespace boost::test_tools::tt_detail
{
template <>
struct print_log_value <Emergence::StandardLayout::FieldArchetype>
{
    void operator () (std::ostream &_output, Emergence::StandardLayout::FieldArchetype const &_value)
    {
        _output << Emergence::StandardLayout::GetFieldArchetypeName (_value);
    }
};

void PrintMapping (std::ostream &_output, Emergence::StandardLayout::Mapping const &_value);

void PrintField (std::ostream &_output, Emergence::StandardLayout::Field const &_value)
{
    _output << "{ archetype: " << Emergence::StandardLayout::GetFieldArchetypeName (_value.GetArchetype ()) <<
            ", offset: " << _value.GetOffset () << ", size: " << _value.GetSize ();

    switch (_value.GetArchetype ())
    {
        case Emergence::StandardLayout::FieldArchetype::BIT:
            _output << ", bitOffset: " << _value.GetBitOffset ();
            break;

        case Emergence::StandardLayout::FieldArchetype::INT:
        case Emergence::StandardLayout::FieldArchetype::UINT:
        case Emergence::StandardLayout::FieldArchetype::FLOAT:
        case Emergence::StandardLayout::FieldArchetype::STRING:
        case Emergence::StandardLayout::FieldArchetype::BLOCK:
            break;

        case Emergence::StandardLayout::FieldArchetype::INSTANCE:
            PrintMapping (_output, _value.GetInstanceMapping ());
            break;
    }

    _output << " }";
}

void PrintMapping (std::ostream &_output, Emergence::StandardLayout::Mapping const &_value)
{
    _output << "{ objectSize: " << _value.GetObjectSize () << ", fields: {";
    Emergence::StandardLayout::Mapping::FieldIterator iterator = _value.Begin ();
    Emergence::StandardLayout::Mapping::FieldIterator end = _value.End ();

    while (iterator != end)
    {
        Emergence::StandardLayout::FieldId fieldId = _value.GetFieldId (iterator);
        _output << "\"" << fieldId << "\": ";
        PrintField (_output, *iterator);
        ++iterator;

        if (iterator != end)
        {
            _output << ", ";
        }
    }

    _output << "} }";
}

template <>
struct print_log_value <Emergence::StandardLayout::Field>
{
    void operator () (std::ostream &_output, Emergence::StandardLayout::Field const &_value)
    {
        PrintField (_output, _value);
    }
};

template <>
struct print_log_value <Emergence::StandardLayout::Mapping>
{
    void operator () (std::ostream &_output, Emergence::StandardLayout::Mapping const &_value)
    {
        PrintMapping (_output, _value);
    }
};

template <>
assertion_result equal_impl (const Emergence::StandardLayout::Mapping &_first,
                             const Emergence::StandardLayout::Mapping &_second)
{
    return Emergence::StandardLayout::Test::operator == (_first, _second);
}

template <>
assertion_result equal_impl (const Emergence::StandardLayout::Field &_first,
                             const Emergence::StandardLayout::Field &_second)
{
    return Emergence::StandardLayout::Test::operator == (_first, _second);
}
}

BOOST_AUTO_TEST_SUITE(MappingBuilder)


struct GenerationDataset
{
    Emergence::StandardLayout::Test::MappingSeed *seed;
    std::string name;

    friend std::ostream &operator << (std::ostream &_output, const GenerationDataset &_dataset)
    {
        return _output << _dataset.name;
    }
};

BOOST_DATA_TEST_CASE (
    GenerateMapping, boost::unit_test::data::monomorphic::collection (
    std::vector <GenerationDataset> {
        {&Emergence::StandardLayout::Test::allBasicTypes, "all basic types"}
    }))
{
    BOOST_REQUIRE_NE (sample.seed, nullptr);
    Emergence::StandardLayout::MappingBuilder builder;
    sample.seed->GrowAndTest (builder);
}

BOOST_AUTO_TEST_SUITE_END()