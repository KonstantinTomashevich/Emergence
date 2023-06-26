#define _CRT_SECURE_NO_WARNINGS

#include <Assert/Assert.hpp>

#include <Container/StringBuilder.hpp>
#include <Container/Vector.hpp>

namespace Emergence::Container
{
StringBuilder::StringBuilder () noexcept
{
    buffer[count] = '\0';
}

StringBuilder &StringBuilder::Reset () noexcept
{
    count = 0u;
    buffer[count] = '\0';
    return *this;
}

const char *StringBuilder::Get () const noexcept
{
    return &buffer.front ();
}

#define SNPRINTF_APPEND(Format)                                                                                        \
    std::size_t spaceLeft = SpaceLeft ();                                                                              \
    if (spaceLeft > 0u)                                                                                                \
    {                                                                                                                  \
        int symbolCount = snprintf (&buffer[count], spaceLeft, (Format), _value);                                      \
        EMERGENCE_ASSERT (symbolCount >= 0);                                                                           \
        count += std::min (spaceLeft, static_cast<std::size_t> (symbolCount));                                         \
    }                                                                                                                  \
                                                                                                                       \
    return *this

StringBuilder &StringBuilder::Append (const char *_value) noexcept
{
    SNPRINTF_APPEND ("%s");
}

StringBuilder &StringBuilder::Append (char *_value) noexcept
{
    SNPRINTF_APPEND ("%s");
}

StringBuilder &StringBuilder::Append (Memory::UniqueString _value) noexcept
{
    return Append (*_value);
}

StringBuilder &StringBuilder::Append (const String &_value) noexcept
{
    return Append (_value.c_str ());
}

StringBuilder &StringBuilder::Append (const std::string_view &_value) noexcept
{
    for (char symbol : _value)
    {
        Append (symbol);
    }

    return *this;
}

StringBuilder &StringBuilder::Append (bool _value) noexcept
{
    return _value ? Append ("true") : Append ("false");
}

StringBuilder &StringBuilder::Append (char _value) noexcept
{
    SNPRINTF_APPEND ("%c");
}

StringBuilder &StringBuilder::Append (short _value) noexcept
{
    SNPRINTF_APPEND ("%hi");
}

StringBuilder &StringBuilder::Append (int _value) noexcept
{
    SNPRINTF_APPEND ("%d");
}

StringBuilder &StringBuilder::Append (long _value) noexcept
{
    SNPRINTF_APPEND ("%ld");
}

StringBuilder &StringBuilder::Append (long long int _value) noexcept
{
    SNPRINTF_APPEND ("%lld");
}

StringBuilder &StringBuilder::Append (unsigned short _value) noexcept
{
    SNPRINTF_APPEND ("%hu");
}

StringBuilder &StringBuilder::Append (unsigned int _value) noexcept
{
    SNPRINTF_APPEND ("%u");
}

StringBuilder &StringBuilder::Append (unsigned long _value) noexcept
{
    SNPRINTF_APPEND ("%lu");
}

StringBuilder &StringBuilder::Append (unsigned long long int _value) noexcept
{
    SNPRINTF_APPEND ("%llu");
}

StringBuilder &StringBuilder::Append (float _value) noexcept
{
    SNPRINTF_APPEND ("%f");
}

StringBuilder &StringBuilder::Append (double _value) noexcept
{
    SNPRINTF_APPEND ("%lf");
}

StringBuilder &StringBuilder::Append (long double _value) noexcept
{
    SNPRINTF_APPEND ("%Lf");
}

StringBuilder &StringBuilder::Append (const FieldPointer &_reflectedField) noexcept
{
    switch (_reflectedField.reflection.GetArchetype ())
    {
    case StandardLayout::FieldArchetype::BIT:
        return Append ((*static_cast<const std::uint8_t *> (_reflectedField.pointer) &
                        (1u << _reflectedField.reflection.GetBitOffset ())) != 0u);

    case StandardLayout::FieldArchetype::INT:
    {
        switch (_reflectedField.reflection.GetSize ())
        {
        case 1u:
            return Append (static_cast<int> (*static_cast<const int8_t *> (_reflectedField.pointer)));
        case 2u:
            return Append (static_cast<int> (*static_cast<const int16_t *> (_reflectedField.pointer)));
        case 4u:
            return Append (*static_cast<const int32_t *> (_reflectedField.pointer));
        case 8u:
            return Append (*static_cast<const int64_t *> (_reflectedField.pointer));
        }

        return Append ("<unknown int size>");
    }

    case StandardLayout::FieldArchetype::UINT:
    {
        switch (_reflectedField.reflection.GetSize ())
        {
        case 1u:
            return Append (static_cast<unsigned> (*static_cast<const std::uint8_t *> (_reflectedField.pointer)));
        case 2u:
            return Append (static_cast<unsigned> (*static_cast<const std::uint16_t *> (_reflectedField.pointer)));
        case 4u:
            return Append (*static_cast<const std::uint32_t *> (_reflectedField.pointer));
        case 8u:
            return Append (*static_cast<const std::uint64_t *> (_reflectedField.pointer));
        }

        return Append ("<unknown std::uint size>");
    }

    case StandardLayout::FieldArchetype::FLOAT:
    {
        switch (_reflectedField.reflection.GetSize ())
        {
        case 4u:
            return Append (*static_cast<const float *> (_reflectedField.pointer));
        case 8u:
            return Append (*static_cast<const double *> (_reflectedField.pointer));
        }

        return Append ("<unknown float size>");
    }

    case StandardLayout::FieldArchetype::STRING:
        return Append (static_cast<const char *> (_reflectedField.pointer));

    case StandardLayout::FieldArchetype::BLOCK:
    {
        const auto *block = static_cast<const std::uint8_t *> (_reflectedField.pointer);
        for (std::size_t index = 0u; index < _reflectedField.reflection.GetSize (); ++index)
        {
            Append (static_cast<unsigned> (block[index]));
            if (index + 1u != _reflectedField.reflection.GetSize ())
            {
                Append (' ');
            }
        }

        return *this;
    }

    case StandardLayout::FieldArchetype::UNIQUE_STRING:
        return Append (*static_cast<const Memory::UniqueString *> (_reflectedField.pointer));

    case StandardLayout::FieldArchetype::NESTED_OBJECT:
        return Append (ObjectPointer {_reflectedField.pointer, _reflectedField.reflection.GetNestedObjectMapping ()});

    case StandardLayout::FieldArchetype::UTF8_STRING:
        return Append (static_cast<const Container::Utf8String *> (_reflectedField.pointer)->c_str ());

    case StandardLayout::FieldArchetype::VECTOR:
    {
        Append ("{ ");
        bool firstField = true;

        for (const std::uint8_t *pointer = UntypedVectorUtility::Begin (_reflectedField.pointer);
             pointer != UntypedVectorUtility::End (_reflectedField.pointer);
             pointer += _reflectedField.reflection.GetVectorItemMapping ().GetObjectSize ())
        {
            if (firstField)
            {
                firstField = false;
            }
            else
            {
                Append (", ");
            }

            Append (ObjectPointer {pointer, _reflectedField.reflection.GetVectorItemMapping ()});
        }

        return Append (" }");
    }

    case StandardLayout::FieldArchetype::PATCH:
        // Patches might be huge, therefore we're not unwrapping them right now.
        return Append ("<patch>");
    }

    return Append ("<unknown field archetype>");
}

StringBuilder &StringBuilder::Append (const ObjectPointer &_reflectedObject) noexcept
{
    Append ("{ ");
    bool firstField = true;

    for (auto iterator = _reflectedObject.reflection.BeginConditional (_reflectedObject.pointer),
              end = _reflectedObject.reflection.EndConditional ();
         iterator != end; ++iterator)
    {
        StandardLayout::Field field = *iterator;
        if (!field.IsProjected ())
        {
            if (firstField)
            {
                firstField = false;
            }
            else
            {
                Append (", ");
            }

            Append (field.GetName (), " = ", FieldPointer {field.GetValue (_reflectedObject.pointer), field});
        }
    }

    Append (" }");
    return *this;
}

std::size_t StringBuilder::GetCount () const noexcept
{
    return count;
}

std::size_t StringBuilder::SpaceLeft () const noexcept
{
    EMERGENCE_ASSERT (count < BUFFER_SIZE);
    return BUFFER_SIZE - count;
}
} // namespace Emergence::Container
