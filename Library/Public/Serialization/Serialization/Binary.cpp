#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <Assert/Assert.hpp>

#include <Container/Optional.hpp>
#include <Container/String.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>

#include <StandardLayout/Patch.hpp>
#include <StandardLayout/PatchBuilder.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Serialization::Binary
{
static void WriteString (std::ostream &_output, const char *_string)
{
    if (_string)
    {
        _output.write (_string, static_cast<std::streamsize> (strlen (_string) + 1u)); // +1 for null terminator.
    }
    else
    {
        // Process null strings as empty strings.
        _output.put ('\0');
    }
}

static Container::Optional<Container::String> ReadString (std::istream &_input)
{
    Container::StringBuilder output;
    while (true)
    {
        char next;
        if (!_input.get (next))
        {
            return std::nullopt;
        }

        if (next == '\0')
        {
            return output.Get ();
        }

        output.Append (next);
    }
}

void SerializeTypeName (std::ostream &_output, Memory::UniqueString _typeName) noexcept
{
    WriteString (_output, *_typeName);
}

Memory::UniqueString DeserializeTypeName (std::istream &_input) noexcept
{
    if (Container::Optional<Container::String> typeName = ReadString (_input))
    {
        return Memory::UniqueString {typeName->c_str()};
    }

    return {};
}

static void SerializePatchValue (std::ostream &_output, const StandardLayout::Field &_field, const void *_value)
{
    switch (_field.GetArchetype ())
    {
    case StandardLayout::FieldArchetype::BIT:
    case StandardLayout::FieldArchetype::INT:
    case StandardLayout::FieldArchetype::UINT:
    case StandardLayout::FieldArchetype::FLOAT:
        _output.write (static_cast<const char *> (_value), static_cast<std::streamsize> (_field.GetSize ()));
        break;

    case StandardLayout::FieldArchetype::UNIQUE_STRING:
        WriteString (_output, **static_cast<const Memory::UniqueString *> (_value));
        break;

    case StandardLayout::FieldArchetype::STRING:
    case StandardLayout::FieldArchetype::BLOCK:
    case StandardLayout::FieldArchetype::NESTED_OBJECT:
    case StandardLayout::FieldArchetype::UTF8_STRING:
    case StandardLayout::FieldArchetype::VECTOR:
    case StandardLayout::FieldArchetype::PATCH:
        // Unsupported.
        EMERGENCE_ASSERT (false);
        break;
    }
}

static bool DeserializePatchValue (std::istream &_input,
                                   const StandardLayout::Field &_field,
                                   StandardLayout::FieldId _fieldId,
                                   StandardLayout::PatchBuilder &_builder)
{
    std::array<std::uint8_t, 8u> buffer;
    if (_field.GetArchetype () != StandardLayout::FieldArchetype::UNIQUE_STRING)
    {
        EMERGENCE_ASSERT (buffer.size () >= _field.GetSize ());
        if (!_input.read (reinterpret_cast<char *> (buffer.data ()), static_cast<std::streamsize> (_field.GetSize ())))
        {
            return false;
        }
    }

    switch (_field.GetArchetype ())
    {
    case StandardLayout::FieldArchetype::BIT:
        _builder.SetBit (_fieldId, block_cast<bool> (buffer));
        break;

    case StandardLayout::FieldArchetype::INT:
        switch (_field.GetSize ())
        {
        case 1u:
            _builder.SetInt8 (_fieldId, block_cast<int8_t> (buffer));
            break;
        case 2u:
            _builder.SetInt16 (_fieldId, block_cast<int16_t> (buffer));
            break;
        case 4u:
            _builder.SetInt32 (_fieldId, block_cast<int32_t> (buffer));
            break;
        case 8u:
            _builder.SetInt64 (_fieldId, block_cast<int64_t> (buffer));
            break;
        }
        break;

    case StandardLayout::FieldArchetype::UINT:
        switch (_field.GetSize ())
        {
        case 1u:
            _builder.SetUInt8 (_fieldId, block_cast<std::uint8_t> (buffer));
            break;
        case 2u:
            _builder.SetUInt16 (_fieldId, block_cast<std::uint16_t> (buffer));
            break;
        case 4u:
            _builder.SetUInt32 (_fieldId, block_cast<std::uint32_t> (buffer));
            break;
        case 8u:
            _builder.SetUInt64 (_fieldId, block_cast<std::uint64_t> (buffer));
            break;
        }
        break;

    case StandardLayout::FieldArchetype::FLOAT:
        switch (_field.GetSize ())
        {
        case 4u:
            _builder.SetFloat (_fieldId, block_cast<float> (buffer));
            break;
        case 8u:
            _builder.SetDouble (_fieldId, block_cast<double> (buffer));
            break;
        }
        break;

    case StandardLayout::FieldArchetype::UNIQUE_STRING:
        if (Container::Optional<Container::String> string = ReadString (_input))
        {
            _builder.SetUniqueString (_fieldId, Memory::UniqueString {string.value ().c_str ()});
        }
        else
        {
            return false;
        }

        break;

    case StandardLayout::FieldArchetype::STRING:
    case StandardLayout::FieldArchetype::BLOCK:
    case StandardLayout::FieldArchetype::NESTED_OBJECT:
    case StandardLayout::FieldArchetype::UTF8_STRING:
    case StandardLayout::FieldArchetype::VECTOR:
    case StandardLayout::FieldArchetype::PATCH:
        // Unsupported.
        EMERGENCE_ASSERT (false);
        return false;
    }

    return true;
}

void SerializePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept
{
    const StandardLayout::Mapping &mapping = _patch.GetTypeMapping ();
    WriteString (_output, *mapping.GetName ());

    const auto changeCount = static_cast<std::uint32_t> (_patch.GetChangeCount ());
    _output.write (reinterpret_cast<const char *> (&changeCount), sizeof (changeCount));

    for (const auto &change : _patch)
    {
        _output.write (reinterpret_cast<const char *> (&change.field), sizeof (change.field));
        StandardLayout::Field field = mapping.GetField (change.field);
        SerializePatchValue (_output, field, change.newValue);
    }
}

bool DeserializePatch (std::istream &_input,
                       void *_outputAddress,
                       const Container::MappingRegistry &_patchableTypesRegistry) noexcept
{
    StandardLayout::PatchBuilder patchBuilder;
    StandardLayout::Mapping mapping;

    if (Container::Optional<Container::String> typeName = ReadString (_input))
    {
        if ((mapping = _patchableTypesRegistry.Get (Memory::UniqueString {*typeName})))
        {
            patchBuilder.Begin (mapping);
        }
        else
        {
            EMERGENCE_LOG (ERROR, "Serialization: Type \"", *typeName, "\" is not patchable!");
            return false;
        }
    }
    else
    {
        return false;
    }

    std::uint32_t changeCount = 0u;
    if (!_input.read (reinterpret_cast<char *> (&changeCount), sizeof (changeCount)))
    {
        return false;
    }

    for (std::uint32_t index = 0u; index < changeCount; ++index)
    {
        StandardLayout::FieldId fieldId;
        if (!_input.read (reinterpret_cast<char *> (&fieldId), sizeof (fieldId)))
        {
            return false;
        }

        StandardLayout::Field field = mapping.GetField (fieldId);
        if (!field.IsHandleValid () || !DeserializePatchValue (_input, field, fieldId, patchBuilder))
        {
            return false;
        }
    }

    *static_cast<StandardLayout::Patch *> (_outputAddress) = patchBuilder.End ();
    return true;
}

void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept
{
    const void *lastBitsetByteAddress = nullptr;
    std::uint8_t lastBitsetByte = 0u;

    for (auto iterator = _mapping.BeginConditional (_object), end = _mapping.EndConditional (); iterator != end;
         ++iterator)
    {
        StandardLayout::Field field = *iterator;
        const void *address = field.GetValue (_object);

        // We extract this from switch for better readability.
        // We need to write last bitset byte if we stopped encountering bits.
        if (lastBitsetByteAddress && field.GetArchetype () != StandardLayout::FieldArchetype::BIT)
        {
            _output.write (reinterpret_cast<const char *> (&lastBitsetByte), sizeof (std::uint8_t));
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
                    _output.write (reinterpret_cast<const char *> (&lastBitsetByte), sizeof (std::uint8_t));
                }

                lastBitsetByteAddress = address;
                lastBitsetByte = 0u;
            }

            lastBitsetByte |= *static_cast<const std::uint8_t *> (address) & (1u << field.GetBitOffset ());
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

        case StandardLayout::FieldArchetype::UTF8_STRING:
            WriteString (_output, static_cast<const Container::Utf8String *> (address)->c_str ());
            break;

        case StandardLayout::FieldArchetype::VECTOR:
        {
            const auto vectorSizeInBytes = static_cast<std::uint32_t> (
                Container::UntypedVectorUtility::End (address) - Container::UntypedVectorUtility::Begin (address));
            EMERGENCE_ASSERT (vectorSizeInBytes % field.GetVectorItemMapping ().GetObjectSize () == 0u);
            const std::uint32_t vectorSize = vectorSizeInBytes / field.GetVectorItemMapping ().GetObjectSize ();

            _output.write (reinterpret_cast<const char *> (&vectorSize),
                           static_cast<std::streamsize> (sizeof (vectorSize)));

            for (const std::uint8_t *pointer = Container::UntypedVectorUtility::Begin (address);
                 pointer != Container::UntypedVectorUtility::End (address);
                 pointer += field.GetVectorItemMapping ().GetObjectSize ())
            {
                SerializeObject (_output, pointer, field.GetVectorItemMapping ());
            }

            break;
        }

        case StandardLayout::FieldArchetype::PATCH:
        {
            SerializePatch (_output, *static_cast<const StandardLayout::Patch *> (address));
            break;
        }
        }
    }

    // If bitset was last field -- write it now.
    if (lastBitsetByteAddress != nullptr)
    {
        _output.write (reinterpret_cast<const char *> (&lastBitsetByte), sizeof (std::uint8_t));
    }
}

bool DeserializeObject (std::istream &_input,
                        void *_object,
                        const StandardLayout::Mapping &_mapping,
                        const Container::MappingRegistry &_patchableTypesRegistry) noexcept
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

            if (static_cast<std::uint8_t> (lastBitsetByte) & (1u << field.GetBitOffset ()))
            {
                *static_cast<std::uint8_t *> (address) |= 1u << field.GetBitOffset ();
            }
            else
            {
                *static_cast<std::uint8_t *> (address) &= ~(1u << field.GetBitOffset ());
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
                static_cast<char *> (address)[field.GetSize () - 1u] = '\0';
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

        case StandardLayout::FieldArchetype::UTF8_STRING:
        {
            if (Container::Optional<Container::String> string = ReadString (_input))
            {
                *static_cast<Container::Utf8String *> (address) = string.value ();
            }
            else
            {
                return false;
            }

            break;
        }

        case StandardLayout::FieldArchetype::VECTOR:
        {
            std::uint32_t vectorSize;
            if (!_input.read (reinterpret_cast<char *> (&vectorSize), sizeof (vectorSize)))
            {
                return false;
            }

            const StandardLayout::Mapping &itemMapping = field.GetVectorItemMapping ();
            Container::UntypedVectorUtility::InitSize (address, vectorSize * itemMapping.GetObjectSize ());

            for (std::uint8_t *pointer = Container::UntypedVectorUtility::Begin (address);
                 pointer != Container::UntypedVectorUtility::End (address); pointer += itemMapping.GetObjectSize ())
            {
                itemMapping.Construct (pointer);
                if (!DeserializeObject (_input, pointer, itemMapping, _patchableTypesRegistry))
                {
                    return false;
                }
            }

            break;
        }

        case StandardLayout::FieldArchetype::PATCH:
        {
            if (!DeserializePatch (_input, address, _patchableTypesRegistry))
            {
                return false;
            }

            break;
        }
        }
    }

    return true;
}
} // namespace Emergence::Serialization::Binary
