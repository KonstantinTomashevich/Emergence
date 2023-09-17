#include <API/Common/BlockCast.hpp>

#include <Memory/Original/UniqueString.hpp>
#include <Memory/UniqueString.hpp>

namespace Emergence::Memory
{
UniqueString::UniqueString () noexcept
{
    new (&data) Original::UniqueString ();
}

UniqueString::UniqueString (const char *_string) noexcept
{
    new (&data) Original::UniqueString (_string);
}

UniqueString::UniqueString (const std::string_view &_string) noexcept
{
    new (&data) Original::UniqueString (_string);
}

const char *UniqueString::operator* () const noexcept
{
    return *block_cast<Original::UniqueString> (data);
}

uintptr_t UniqueString::Hash () const noexcept
{
    return block_cast<Original::UniqueString> (data).Hash ();
}

bool UniqueString::operator== (const UniqueString &_other) const noexcept
{
    return block_cast<Original::UniqueString> (data) == block_cast<Original::UniqueString> (_other.data);
}

bool UniqueString::operator!= (const UniqueString &_other) const noexcept
{
    return !(*this == _other);
}

std::ostream &operator<< (std::ostream &_output, const UniqueString &_string) noexcept
{
    return _output << (*_string ? *_string : "<empty string>");
}
} // namespace Emergence::Memory
