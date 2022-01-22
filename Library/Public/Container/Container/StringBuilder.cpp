#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

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

#define SNPRINTF_APPEND(Format, Data)                                                                                  \
    std::size_t spaceLeft = SpaceLeft ();                                                                              \
    if (spaceLeft > 0u)                                                                                                \
    {                                                                                                                  \
        int count = snprintf (&buffer[end], spaceLeft, (Format), Data);                                                \
        assert (count >= 0);                                                                                           \
        end += std::min (spaceLeft, static_cast<std::size_t> (count));                                                 \
    }                                                                                                                  \
                                                                                                                       \
    return *this

StringBuilder &StringBuilder::Append (const char *_value) noexcept
{
    SNPRINTF_APPEND ("%s", _value);
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
    SNPRINTF_APPEND ("%s", _value.data ());
}

StringBuilder &StringBuilder::Append (char _value) noexcept
{
    SNPRINTF_APPEND ("%c", _value);
}

StringBuilder &StringBuilder::Append (int _value) noexcept
{
    SNPRINTF_APPEND ("%d", _value);
}

StringBuilder &StringBuilder::Append (long _value) noexcept
{
    SNPRINTF_APPEND ("%ld", _value);
}

StringBuilder &StringBuilder::Append (long long int _value) noexcept
{
    SNPRINTF_APPEND ("%lld", _value);
}

StringBuilder &StringBuilder::Append (unsigned int _value) noexcept
{
    SNPRINTF_APPEND ("%u", _value);
}

StringBuilder &StringBuilder::Append (unsigned long _value) noexcept
{
    SNPRINTF_APPEND ("%lu", _value);
}

StringBuilder &StringBuilder::Append (unsigned long long int _value) noexcept
{
    SNPRINTF_APPEND ("%llu", _value);
}

StringBuilder &StringBuilder::Append (float _value) noexcept
{
    SNPRINTF_APPEND ("%f", _value);
}

StringBuilder &StringBuilder::Append (double _value) noexcept
{
    SNPRINTF_APPEND ("%lf", _value);
}

StringBuilder &StringBuilder::Append (long double _value) noexcept
{
    SNPRINTF_APPEND ("%Lf", _value);
}

std::size_t StringBuilder::SpaceLeft () const noexcept
{
    assert (end < BUFFER_SIZE);
    return BUFFER_SIZE - end;
}
} // namespace Emergence::Container
