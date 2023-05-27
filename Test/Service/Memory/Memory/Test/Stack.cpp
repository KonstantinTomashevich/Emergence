#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <Memory/Stack.hpp>
#include <Memory/Test/Helpers.hpp>
#include <Memory/Test/Stack.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Test
{
bool StackTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Memory::Test

using namespace Emergence::Memory::Literals;
using namespace Emergence::Memory::Test;

BEGIN_SUITE (Stack)

TEST_CASE (AcquireNoOverlap)
{
    const char first[] = "Hello, world!\0";
    const char second[] = "Lets check the overwrite!\0";

    Emergence::Memory::Stack stack {GetUniqueAllocationGroup (), 1024u};
    char *firstTarget = static_cast<char *> (stack.Acquire (sizeof (first), 1u));
    char *secondTarget = static_cast<char *> (stack.Acquire (sizeof (second), 1u));

    strcpy (firstTarget, first);
    strcpy (secondTarget, second);

    CHECK (strcmp (first, firstTarget) == 0);
    CHECK (strcmp (second, secondTarget) == 0);
}

TEST_CASE (AcquireAlignment)
{
    Emergence::Memory::Stack stack {GetUniqueAllocationGroup (), 1024u};
#define CHECK_ALIGNMENT(Size, Alignment)                                                                               \
    CHECK_EQUAL (reinterpret_cast<std::uintptr_t> (stack.Acquire (Size, Alignment)) % (Alignment), 0u)

    CHECK_ALIGNMENT (5u, 1u);
    CHECK_ALIGNMENT (32u, 8u);
    CHECK_ALIGNMENT (20u, 8u);
    CHECK_ALIGNMENT (13u, 4u);
    CHECK_ALIGNMENT (19u, 4u);
    CHECK_ALIGNMENT (32u, 8u);
    CHECK_ALIGNMENT (128u, 32u);
    CHECK_ALIGNMENT (5u, 1u);
    CHECK_ALIGNMENT (3u, 2u);
    CHECK_ALIGNMENT (48u, 8u);

#undef CHECK_ALIGNMENT
}

TEST_CASE (ClearAndReuse)
{
    Emergence::Memory::Stack stack {GetUniqueAllocationGroup (), 1024u};
    void *firstRecord = stack.Acquire (225u, sizeof (std::uintptr_t));
    stack.Clear ();

    void *secondRecord = stack.Acquire (123u, sizeof (std::uintptr_t));
    CHECK_EQUAL (firstRecord, secondRecord);
}

TEST_CASE (ReleaseAndReuse)
{
    Emergence::Memory::Stack stack {GetUniqueAllocationGroup (), 1024u};
    [[maybe_unused]] void *historicalShift = stack.Acquire (225u, sizeof (std::uintptr_t));
    const void *cachedHead = stack.Head ();

    constexpr std::size_t FIRST_RECORD_SIZE = 128u;
    constexpr std::size_t SECOND_RECORD_SIZE = 128u;
    constexpr std::size_t THIRD_RECORD_SIZE = 128u;

    void *firstRecord = stack.Acquire (FIRST_RECORD_SIZE, sizeof (std::uintptr_t));
    [[maybe_unused]] void *secondRecord = stack.Acquire (SECOND_RECORD_SIZE, sizeof (std::uintptr_t));
    void *thirdRecord = stack.Acquire (THIRD_RECORD_SIZE, sizeof (std::uintptr_t));

    stack.Release (cachedHead);
    CHECK_EQUAL (stack.Head (), cachedHead);
    CHECK_EQUAL (stack.Acquire (FIRST_RECORD_SIZE + SECOND_RECORD_SIZE, sizeof (std::uintptr_t)), firstRecord);
    CHECK_EQUAL (stack.Acquire (THIRD_RECORD_SIZE, sizeof (std::uintptr_t)), thirdRecord);
}

TEST_CASE (FreeSize)
{
    constexpr std::size_t STACK_SIZE = 1024u;
    Emergence::Memory::Stack stack {GetUniqueAllocationGroup (), STACK_SIZE};
    CHECK_EQUAL (stack.GetFreeSize (), STACK_SIZE);

    [[maybe_unused]] void *stub1 = stack.Acquire (6u, 1u);
    CHECK_EQUAL (stack.GetFreeSize (), STACK_SIZE - 6u);
    const void *cachedHead = stack.Head ();

    [[maybe_unused]] void *stub2 = stack.Acquire (17u, 8u);
    CHECK_EQUAL (stack.GetFreeSize (), STACK_SIZE - 25u);

    [[maybe_unused]] void *stub3 = stack.Acquire (12u, 4u);
    CHECK_EQUAL (stack.GetFreeSize (), STACK_SIZE - 40u);

    stack.Release (cachedHead);
    CHECK_EQUAL (stack.GetFreeSize (), STACK_SIZE - 6u);

    stack.Clear ();
    CHECK_EQUAL (stack.GetFreeSize (), STACK_SIZE);
}

TEST_CASE (Move)
{
    Emergence::Memory::Stack stack {GetUniqueAllocationGroup (), 1024u};
    [[maybe_unused]] void *firstRecord = stack.Acquire (32u, sizeof (std::uintptr_t));
    const void *cachedHead = stack.Head ();

    Emergence::Memory::Stack movedStack {std::move (stack)};
    CHECK_EQUAL (movedStack.Head (), cachedHead);

    // Acquire to check usability.
    [[maybe_unused]] void *secondRecord = movedStack.Acquire (128u, sizeof (std::uintptr_t));
}

TEST_CASE (MoveAssign)
{
    Emergence::Memory::Stack stack {GetUniqueAllocationGroup (), 1024u};
    Emergence::Memory::Stack anotherStack {GetUniqueAllocationGroup (), 1024u};

    [[maybe_unused]] void *firstRecord = stack.Acquire (32u, sizeof (std::uintptr_t));
    const void *cachedHead = stack.Head ();

    anotherStack = std::move (stack);
    CHECK_EQUAL (anotherStack.Head (), cachedHead);

    // Acquire to check usability.
    [[maybe_unused]] void *secondRecord = anotherStack.Acquire (128u, sizeof (std::uintptr_t));
}

TEST_CASE (Profiling)
{
    constexpr std::size_t STACK_CAPACITY = 1024u;
    constexpr std::size_t FIRST_ALLOCATION = 32u;
    constexpr std::size_t SECOND_ALLOCATION = 128u;

    Emergence::Memory::Profiler::AllocationGroup group = GetUniqueAllocationGroup ();
    Emergence::Memory::Stack stack {group, STACK_CAPACITY};

    CHECK_EQUAL (group.GetReserved (), STACK_CAPACITY);
    CHECK_EQUAL (group.GetAcquired (), 0u);
    CHECK_EQUAL (group.GetTotal (), STACK_CAPACITY);

    [[maybe_unused]] void *first = stack.Acquire (FIRST_ALLOCATION, sizeof (std::uintptr_t));
    const void *headAfterFirst = stack.Head ();

    CHECK_EQUAL (group.GetReserved (), STACK_CAPACITY - FIRST_ALLOCATION);
    CHECK_EQUAL (group.GetAcquired (), FIRST_ALLOCATION);
    CHECK_EQUAL (group.GetTotal (), STACK_CAPACITY);

    [[maybe_unused]] void *second = stack.Acquire (SECOND_ALLOCATION, sizeof (std::uintptr_t));
    CHECK_EQUAL (group.GetReserved (), STACK_CAPACITY - FIRST_ALLOCATION - SECOND_ALLOCATION);
    CHECK_EQUAL (group.GetAcquired (), FIRST_ALLOCATION + SECOND_ALLOCATION);
    CHECK_EQUAL (group.GetTotal (), STACK_CAPACITY);

    stack.Release (headAfterFirst);
    CHECK_EQUAL (group.GetReserved (), STACK_CAPACITY - FIRST_ALLOCATION);
    CHECK_EQUAL (group.GetAcquired (), FIRST_ALLOCATION);
    CHECK_EQUAL (group.GetTotal (), STACK_CAPACITY);

    stack.Clear ();
    CHECK_EQUAL (group.GetReserved (), STACK_CAPACITY);
    CHECK_EQUAL (group.GetAcquired (), 0u);
    CHECK_EQUAL (group.GetTotal (), STACK_CAPACITY);
}

END_SUITE
