#define _CRT_SECURE_NO_WARNINGS

#include <cassert>
#include <cstring>

#include <Container/StringBuilder.hpp>

namespace Emergence::Container
{
StringBuilder::StringBuilder () noexcept
{
    buffer[end] = '\0';
}

StringBuilder &StringBuilder::Reset () noexcept
{
    end = 0u;
    buffer[end] = '\0';
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
        int count = snprintf (&buffer[end], spaceLeft, (Format), _value);                                              \
        assert (count >= 0);                                                                                           \
        end += std::min (spaceLeft, static_cast<std::size_t> (count));                                                 \
    }                                                                                                                  \
                                                                                                                       \
    return *this

StringBuilder &StringBuilder::Append (const char *_value) noexcept
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
    return Append (_value.data ());
}

StringBuilder &StringBuilder::Append (bool _value) noexcept
{
    return _value ? Append ("true") : Append ("false");
}

StringBuilder &StringBuilder::Append (char _value) noexcept
{
    SNPRINTF_APPEND ("%c");
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
        return Append (*static_cast<const uint8_t *> (_reflectedField.pointer) != 0u);

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
            return Append (static_cast<unsigned> (*static_cast<const uint8_t *> (_reflectedField.pointer)));
        case 2u:
            return Append (static_cast<unsigned> (*static_cast<const uint16_t *> (_reflectedField.pointer)));
        case 4u:
            return Append (*static_cast<const uint32_t *> (_reflectedField.pointer));
        case 8u:
            return Append (*static_cast<const uint64_t *> (_reflectedField.pointer));
        }

        return Append ("<unknown uint size>");
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
        const auto *block = static_cast<const uint8_t *> (_reflectedField.pointer);
        for (size_t index = 0u; index < _reflectedField.reflection.GetSize (); ++index)
        {
            Append (static_cast<unsigned> (block[index]));
        }

        return *this;
    }

    case StandardLayout::FieldArchetype::UNIQUE_STRING:
        return Append (*static_cast<const Memory::UniqueString *> (_reflectedField.pointer));

    case StandardLayout::FieldArchetype::NESTED_OBJECT:
        return Append (ObjectPointer {_reflectedField.pointer, _reflectedField.reflection.GetNestedObjectMapping ()});
    }

    return Append ("<unknown field archetype>");
}

StringBuilder &StringBuilder::Append (const ObjectPointer &_reflectedObject) noexcept
{
    Append ("{ ");
    for (auto iterator = _reflectedObject.reflection.Begin (); iterator != _reflectedObject.reflection.End ();
         ++iterator)
    {
        StandardLayout::Field field = *iterator;
        if (iterator != _reflectedObject.reflection.Begin ())
        {
            Append (", ");
        }

        // Ignore projected fields.
        if (!strchr (*field.GetName (), StandardLayout::PROJECTION_NAME_SEPARATOR))
        {
            Append (field.GetName (), " = ", FieldPointer {field.GetValue (_reflectedObject.pointer), field});
        }
    }

    Append (" }");
    return *this;
}

std::size_t StringBuilder::SpaceLeft () const noexcept
{
    assert (end < BUFFER_SIZE);
    return BUFFER_SIZE - end;
}
} // namespace Emergence::Container
