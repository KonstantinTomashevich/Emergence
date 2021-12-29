#define _CRT_SECURE_NO_WARNINGS

#include <cassert>
#include <unordered_set>

#include <Memory/Original/UniqueString.hpp>
#include <Memory/Stack.hpp>
#include <Memory/UnorderedPool.hpp>

namespace Emergence::Memory::Original
{
static const char *RegisterValue (const std::string_view &_value)
{
    if (_value.empty ())
    {
        return nullptr;
    }

    // In most cases 3-4 or even 1 stack would be enough, but we would like to
    // avoid creating additional pages, therefore we use this page capacity.
    constexpr const std::size_t STACK_POOL_PAGE_CAPACITY = 16u;

    constexpr const std::size_t STRING_STACK_SIZE = 1u << 20u; // 1 MB.

    // For flexibility, we dynamically allocate new string stacks instead of using one big stack.
    static UnorderedPool stacksPool {sizeof (Stack), STACK_POOL_PAGE_CAPACITY};

    // Usually, unique strings are quite small (<100 characters), therefore it's ok to use
    // only last allocated stack instead of trying to insert into all allocated stacks.
    static auto *lastStack = new (stacksPool.Acquire ()) Stack (STRING_STACK_SIZE);

    // Right now we use unordered set for simplicity.
    static std::unordered_set<std::string_view> stringRegister;

    auto iterator = stringRegister.find (_value);
    if (iterator == stringRegister.end ())
    {
        assert (_value.size () + 1u < STRING_STACK_SIZE);
        if (lastStack->GetFreeSize () < _value.size () + 1u)
        {
            // Current stack is full, we need a new one.
            lastStack = new (stacksPool.Acquire ()) Stack (STRING_STACK_SIZE);
        }

        char *space = static_cast<char *> (lastStack->Acquire (_value.size () + 1u, 1u));
        strcpy (space, _value.data ());
        auto [insertionIterator, result] = stringRegister.emplace (space);
        assert (result);
        return insertionIterator->data ();
    }

    return iterator->data ();
}

Original::UniqueString::UniqueString (const char *_string) noexcept : UniqueString (std::string_view {_string})
{
}

Original::UniqueString::UniqueString (const std::string_view &_string) noexcept : value (RegisterValue (_string))
{
}

const char *Original::UniqueString::operator* () const noexcept
{
    return value;
}

uintptr_t Original::UniqueString::Hash () const noexcept
{
    return reinterpret_cast<uintptr_t> (value);
}

bool Original::UniqueString::operator== (const UniqueString &_other) const
{
    return value == _other.value;
}

bool Original::UniqueString::operator!= (const UniqueString &_other) const
{
    return !(*this == _other);
}
} // namespace Emergence::Memory::Original
