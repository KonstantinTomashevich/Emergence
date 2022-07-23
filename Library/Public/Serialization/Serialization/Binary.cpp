#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

#include <Container/Optional.hpp>
#include <Container/String.hpp>

#include <Serialization/Binary.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Serialization::Binary
{
static void WriteString (std::ostream &_output, const char *_string)
{
    _output.write (_string, static_cast<std::streamsize> (strlen (_string) + 1u)); // +1 for null terminator.
}

static Container::Optional<Container::String> ReadString (std::istream &_input)
{
    Container::String output;
    while (true)
    {
        char next;
        if (!_input.get (next))
        {
            return std::nullopt;
        }

        if (next == '\0')
        {
            return output;
        }

        output += next;
    }
};

void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept
{
    const void *lastBitsetByteAddress = nullptr;
    uint8_t lastBitsetByte = 0u;

    for (auto iterator = _mapping.BeginConditional (_object), end = _mapping.EndConditional (); iterator != end;
         ++iterator)
    {
        StandardLayout::Field field = *iterator;
        const void *address = field.GetValue (_object);

        // We extract this from switch for better readability.
        // We need to write last bitset byte if we stopped encountering bits.
        if (lastBitsetByteAddress && field.GetArchetype () != StandardLayout::FieldArchetype::BIT)
        {
            _output.write (reinterpret_cast<const char *> (&lastBitsetByte), sizeof (uint8_t));
            lastBitsetByteAddress = nullptr;
        }

        switch (field.GetArchetype ())
        {
        case StandardLayout::FieldArchetype::BIT:
        {
            if (lastBitsetByteAddress != address)
            {
                if (lastBitsetByteAddress != address && lastBitsetByteAddress != nullptr)
                {
                    // We're starting new bitset byte: write older one.
                    _output.write (reinterpret_cast<const char *> (&lastBitsetByte), sizeof (uint8_t));
                }

                lastBitsetByteAddress = address;
                lastBitsetByte = 0u;
            }

            lastBitsetByte |= *static_cast<const uint8_t *> (address) & (1u << field.GetBitOffset ());
            break;
        }

        case StandardLayout::FieldArchetype::INT:
        case StandardLayout::FieldArchetype::UINT:
        case StandardLayout::FieldArchetype::FLOAT:
        case StandardLayout::FieldArchetype::BLOCK:
            _output.write (static_cast<const char *> (address), static_cast<std::streamsize> (field.GetSize ()));
            break;

        case StandardLayout::FieldArchetype::STRING:
            WriteString (_output, static_cast<const char *> (address));
            break;

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
            WriteString (_output, **static_cast<const Memory::UniqueString *> (address));
            break;

        case StandardLayout::FieldArchetype::NESTED_OBJECT:
            // We do nothing for nested objects, because all of their fields are projected.
            break;
        }
    }

    // If bitset was last field -- write it now.
    if (lastBitsetByteAddress != nullptr)
    {
        _output.write (reinterpret_cast<const char *> (&lastBitsetByte), sizeof (uint8_t));
    }
}

bool DeserializeObject (std::istream &_input, void *_object, const StandardLayout::Mapping &_mapping) noexcept
{
    const void *lastBitsetByteAddress = nullptr;
    char lastBitsetByte = 0u;

    for (auto iterator = _mapping.BeginConditional (_object), end = _mapping.EndConditional (); iterator != end;
         ++iterator)
    {
        StandardLayout::Field field = *iterator;
        void *address = field.GetValue (_object);

        switch (field.GetArchetype ())
        {
        case StandardLayout::FieldArchetype::BIT:
        {
            if (lastBitsetByteAddress != address)
            {
                lastBitsetByteAddress = address;
                if (!_input.get (lastBitsetByte))
                {
                    return false;
                }
            }

            if (static_cast<uint8_t> (lastBitsetByte) & (1u << field.GetBitOffset ()))
            {
                *static_cast<uint8_t *> (address) |= 1u << field.GetBitOffset ();
            }
            else
            {
                *static_cast<uint8_t *> (address) &= ~(1u << field.GetBitOffset ());
            }

            break;
        }

        case StandardLayout::FieldArchetype::INT:
        case StandardLayout::FieldArchetype::UINT:
        case StandardLayout::FieldArchetype::FLOAT:
        case StandardLayout::FieldArchetype::BLOCK:
            if (!_input.read (static_cast<char *> (address), static_cast<std::streamsize> (field.GetSize ())))
            {
                return false;
            }
            break;

        case StandardLayout::FieldArchetype::STRING:
        {
            if (Container::Optional<Container::String> string = ReadString (_input))
            {
                strncpy (static_cast<char *> (address), string.value ().c_str (), field.GetSize () - 1u);
            }
            else
            {
                return false;
            }

            break;
        }

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
        {
            if (Container::Optional<Container::String> string = ReadString (_input))
            {
                *static_cast<Memory::UniqueString *> (address) = Memory::UniqueString {string.value ().c_str ()};
            }
            else
            {
                return false;
            }

            break;
        }

        case StandardLayout::FieldArchetype::NESTED_OBJECT:
            // We do nothing for nested objects, because all of their fields are projected.
            break;
        }
    }

    return true;
}

void SerializePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept
{
    const StandardLayout::Mapping &mapping = _patch.GetTypeMapping ();
    const auto changeCount = static_cast<uint32_t> (_patch.GetChangeCount ());
    _output.write (reinterpret_cast<const char *> (&changeCount), sizeof (changeCount));

    for (const auto &change : _patch)
    {
        _output.write (reinterpret_cast<const char *> (&change.field), sizeof (change.field));
        StandardLayout::Field field = mapping.GetField (change.field);

        switch (field.GetArchetype ())
        {
        case StandardLayout::FieldArchetype::BIT:
        case StandardLayout::FieldArchetype::INT:
        case StandardLayout::FieldArchetype::UINT:
        case StandardLayout::FieldArchetype::FLOAT:
            _output.write (static_cast<const char *> (change.newValue),
                           static_cast<std::streamsize> (field.GetSize ()));
            break;

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
            WriteString (_output, **static_cast<const Memory::UniqueString *> (change.newValue));
            break;

        case StandardLayout::FieldArchetype::STRING:
        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
            // Unsupported.
            assert (false);
            break;
        }
    }
}

bool DeserializePatch (std::istream &_input,
                       StandardLayout::PatchBuilder &_builder,
                       const StandardLayout::Mapping &_mapping) noexcept
{
    _builder.Begin (_mapping);
    std::uint32_t changeCount = 0u;

    if (!_input.read (reinterpret_cast<char *> (&changeCount), sizeof (changeCount)))
    {
        return false;
    }

    std::array<uint8_t, 8u> buffer;
    for (std::uint32_t index = 0u; index < changeCount; ++index)
    {
        StandardLayout::FieldId fieldId;
        if (!_input.read (reinterpret_cast<char *> (&fieldId), sizeof (fieldId)))
        {
            return false;
        }

        StandardLayout::Field field = _mapping.GetField (fieldId);
        if (!field.IsHandleValid ())
        {
            return false;
        }

        if (field.GetArchetype () != StandardLayout::FieldArchetype::UNIQUE_STRING)
        {
            assert (buffer.size () >= field.GetSize ());
            if (!_input.read (reinterpret_cast<char *> (buffer.data ()),
                              static_cast<std::streamsize> (field.GetSize ())))
            {
                return false;
            }
        }

        switch (field.GetArchetype ())
        {
        case StandardLayout::FieldArchetype::BIT:
            _builder.SetBit (fieldId, block_cast<bool> (buffer));
            break;

        case StandardLayout::FieldArchetype::INT:
            switch (field.GetSize ())
            {
            case 1u:
                _builder.SetInt8 (fieldId, block_cast<int8_t> (buffer));
                break;
            case 2u:
                _builder.SetInt16 (fieldId, block_cast<int16_t> (buffer));
                break;
            case 4u:
                _builder.SetInt32 (fieldId, block_cast<int32_t> (buffer));
                break;
            case 8u:
                _builder.SetInt64 (fieldId, block_cast<int64_t> (buffer));
                break;
            }
            break;

        case StandardLayout::FieldArchetype::UINT:
            switch (field.GetSize ())
            {
            case 1u:
                _builder.SetUInt8 (fieldId, block_cast<uint8_t> (buffer));
                break;
            case 2u:
                _builder.SetUInt16 (fieldId, block_cast<uint16_t> (buffer));
                break;
            case 4u:
                _builder.SetUInt32 (fieldId, block_cast<uint32_t> (buffer));
                break;
            case 8u:
                _builder.SetUInt64 (fieldId, block_cast<uint64_t> (buffer));
                break;
            }
            break;

        case StandardLayout::FieldArchetype::FLOAT:
            switch (field.GetSize ())
            {
            case 4u:
                _builder.SetFloat (fieldId, block_cast<float> (buffer));
                break;
            case 8u:
                _builder.SetDouble (fieldId, block_cast<double> (buffer));
                break;
            }
            break;

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
            if (Container::Optional<Container::String> string = ReadString (_input))
            {
                _builder.SetUniqueString (fieldId, Memory::UniqueString {string.value ().c_str ()});
            }
            else
            {
                return false;
            }

            break;

        case StandardLayout::FieldArchetype::STRING:
        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
            // Unsupported.
            assert (false);
            return false;
        }
    }

    return true;
}
} // namespace Emergence::Serialization::Binary
