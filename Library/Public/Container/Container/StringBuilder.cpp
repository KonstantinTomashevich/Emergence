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

std::size_t StringBuilder::SpaceLeft () const noexcept
{
    assert (end < BUFFER_SIZE);
    return BUFFER_SIZE - end;
}
} // namespace Emergence::Container
