#define _CRT_SECURE_NO_WARNINGS

#include <Container/String.hpp>

#include <Serialization/Binary.hpp>

namespace Emergence::Serialization
{
void Binary::SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept
{
    auto writeString = [&_output] (const char *_string)
    {
        _output.write (_string, static_cast<std::streamsize> (strlen (_string) + 1u)); // +1 for null terminator.
    };

    const void *lastBitsetByteAddress = nullptr;
    uint8_t lastBitsetByte = 0u;

    for (auto iterator = _mapping.BeginConditional (_object), end = _mapping.EndConditional (); iterator != end;
         ++iterator)
    {
        StandardLayout::Field field = *iterator;
        const void *address = field.GetValue (_object);

        // We extract this from switch for better readability.
        // We need to write last bitset byte if we stopped encountering bits.
        if (lastBitsetByteAddress && field.GetArchetype() != StandardLayout::FieldArchetype::BIT)
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
            writeString (static_cast<const char *> (address));
            break;

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
            writeString (**static_cast<const Memory::UniqueString *> (address));
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

bool Binary::DeserializeObject (std::istream &_input, void *_object, const StandardLayout::Mapping &_mapping) noexcept
{
    auto readString = [&_input] (Container::String &_output)
    {
        while (true)
        {
            char next;
            if (!_input.get (next))
            {
                return false;
            }

            if (next == '\0')
            {
                return true;
            }

            _output += next;
        }
    };

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
            Container::String temporary;
            if (!readString (temporary))
            {
                return false;
            }

            strncpy (static_cast<char *> (address), temporary.c_str (), field.GetSize () - 1u);
            break;
        }

        case StandardLayout::FieldArchetype::UNIQUE_STRING:
        {
            Container::String temporary;
            if (!readString (temporary))
            {
                return false;
            }

            *static_cast<Memory::UniqueString *> (address) = Memory::UniqueString {temporary.c_str ()};
            break;
        }

        case StandardLayout::FieldArchetype::NESTED_OBJECT:
            // We do nothing for nested objects, because all of their fields are projected.
            break;
        }
    }

    return true;
}
} // namespace Emergence::Serialization
