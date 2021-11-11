#define _CRT_SECURE_NO_WARNINGS

#include <cassert>
#include <unordered_set>

#include <Memory/StackAllocator.hpp>

#include <StandardLayout/MappingBuilder.hpp>

#include <String/ConstReference.hpp>

#include <SyntaxSugar/MappingRegistration.hpp>

namespace Emergence::String
{
static const char *RegisterValue (const std::string_view &_value)
{
    if (_value.empty ())
    {
        return nullptr;
    }

    constexpr const std::size_t STRING_BUFFER_MAX_SIZE = 5u * 1024u * 1024u;
    static Memory::StackAllocator buffer {STRING_BUFFER_MAX_SIZE};

    // Right now we use std hash map for simplicity.
    static std::unordered_set<std::string_view> stringRegister;

    auto iterator = stringRegister.find (_value);
    if (iterator == stringRegister.end ())
    {
        char *space = static_cast<char *> (buffer.Acquire (_value.size () + 1u));
        strcpy (space, _value.data ());
        auto [insertionIterator, result] = stringRegister.emplace (space);
        assert (result);
        return insertionIterator->data ();
    }

    return iterator->data ();
}

ConstReference::ConstReference (const char *_value) noexcept : ConstReference (std::string_view {_value})
{
}

ConstReference::ConstReference (const std::string_view &_value) noexcept : value (RegisterValue (_value))
{
}

const char *ConstReference::Value () const noexcept
{
    return value;
}

const ConstReference::Reflection &ConstReference::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (String::ConstReference)
        EMERGENCE_MAPPING_REGISTER_POINTER_AS_UINT (value)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::String
