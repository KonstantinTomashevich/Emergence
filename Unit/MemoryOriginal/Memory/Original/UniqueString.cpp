#define _CRT_SECURE_NO_WARNINGS

#include <atomic>

#include <Assert/Assert.hpp>

#include <Container/HashSet.hpp>

#include <Memory/Original/UniqueString.hpp>
#include <Memory/Stack.hpp>
#include <Memory/UnorderedPool.hpp>

#include <Threading/AtomicFlagGuard.hpp>

namespace Emergence::Memory::Original
{
static const char MEMORY_PROFILING_GROUP_ID[] = "UniqueString";

static const char *RegisterValue (const std::string_view &_value)
{
    if (_value.empty ())
    {
        return nullptr;
    }

    // We can not just compare pointers because of several reasons:
    // - Optimizer is not guaranteed to merge constants.
    // - Constant with the same value might be loaded from other source,
    //   for example memory profiler recording file or from dynamic linking library.
    if (_value == MEMORY_PROFILING_GROUP_ID)
    {
        return MEMORY_PROFILING_GROUP_ID;
    }

    // We need to make sure that register is not accessed from multiple threads
    // simultaneously because it could result in race condition.
    static std::atomic_flag registrationLock;
    AtomicFlagGuard guard {registrationLock};

    // Code is moved into separate block to make sure that static constructors,
    // related to memory allocation, will not be called if condition above returned.
    {
        // In most cases 3-4 or even 1 stack would be enough, but we would like to
        // avoid creating additional pages, therefore we use this page capacity.
        constexpr const std::size_t STACK_POOL_PAGE_CAPACITY = 16u;

        constexpr const std::size_t STRING_STACK_SIZE = 1u << 20u; // 1 MB.

        static Profiler::AllocationGroup allocationGroup {Profiler::AllocationGroup::Root (),
                                                          Memory::UniqueString {MEMORY_PROFILING_GROUP_ID}};

        // For flexibility, we dynamically allocate new string stacks instead of using one big stack.
        static UnorderedPool stacksPool {allocationGroup, sizeof (Stack), alignof (Stack), STACK_POOL_PAGE_CAPACITY};

        // Usually, unique strings are quite small (<100 characters), therefore it's ok to use
        // only last allocated stack instead of trying to insert into all allocated stacks.
        static auto *lastStack = new (stacksPool.Acquire ()) Stack (allocationGroup, STRING_STACK_SIZE);

        // Right now we use hash set for simplicity.
        static Container::HashSet<std::string_view> stringRegister {allocationGroup};

        auto iterator = stringRegister.find (_value);
        if (iterator == stringRegister.end ())
        {
            EMERGENCE_ASSERT (_value.size () + 1u < STRING_STACK_SIZE);
            if (lastStack->GetFreeSize () < _value.size () + 1u)
            {
                // Current stack is full, we need a new one.
                lastStack = new (stacksPool.Acquire ()) Stack (allocationGroup, STRING_STACK_SIZE);
            }

            char *space = static_cast<char *> (lastStack->Acquire (_value.size () + 1u, 1u));
            for (std::size_t index = 0u; index < _value.size (); ++index)
            {
                space[index] = _value[index];
            }

            space[_value.size ()] = '\0';
            auto [insertionIterator, result] = stringRegister.emplace (space);
            EMERGENCE_ASSERT (result);
            return insertionIterator->data ();
        }

        return iterator->data ();
    }
}

UniqueString::UniqueString (const char *_string) noexcept
    : UniqueString (std::string_view {_string})
{
}

UniqueString::UniqueString (const std::string_view &_string) noexcept
    : value (RegisterValue (_string))
{
}

const char *UniqueString::operator* () const noexcept
{
    return value;
}

uintptr_t UniqueString::Hash () const noexcept
{
    return reinterpret_cast<std::uintptr_t> (value);
}

bool UniqueString::operator== (const UniqueString &_other) const
{
    return value == _other.value;
}

bool UniqueString::operator!= (const UniqueString &_other) const
{
    return !(*this == _other);
}
} // namespace Emergence::Memory::Original
