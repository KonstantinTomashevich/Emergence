#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <Memory/Stack.hpp>
#include <Memory/Test/Stack.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Test
{
bool StackTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Memory::Test

BEGIN_SUITE (Stack)

TEST_CASE (AcquireNoOverlap)
{
    const char first[] = "Hello, world!\0";
    const char second[] = "Lets check the overwrite!\0";

    Emergence::Memory::Stack stack {1024u};
    char *firstTarget = static_cast<char *> (stack.Acquire (sizeof (first), 1u));
    char *secondTarget = static_cast<char *> (stack.Acquire (sizeof (second), 1u));

    strcpy (firstTarget, first);
    strcpy (secondTarget, second);

    CHECK (strcmp (first, firstTarget) == 0);
    CHECK (strcmp (second, secondTarget) == 0);
}

TEST_CASE (AcquireAlignment)
{
    Emergence::Memory::Stack stack {1024u};
#define CHECK_ALIGNMENT(Size, Alignment)                                                                               \
    CHECK_EQUAL (reinterpret_cast<uintptr_t> (stack.Acquire (Size, Alignment)) % (Alignment), 0u)

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
    Emergence::Memory::Stack stack {1024u};
    void *firstRecord = stack.Acquire (225u);
    stack.Clear ();

    void *secondRecord = stack.Acquire (123u);
    CHECK_EQUAL (firstRecord, secondRecord);
}

TEST_CASE (ReleaseAndReuse)
{
    Emergence::Memory::Stack stack {1024u};
    [[maybe_unused]] void *historicalShift = stack.Acquire (225u);
    const void *cachedHead = stack.Head ();

    constexpr size_t FIRST_RECORD_SIZE = 128u;
    constexpr size_t SECOND_RECORD_SIZE = 128u;
    constexpr size_t THIRD_RECORD_SIZE = 128u;

    void *firstRecord = stack.Acquire (FIRST_RECORD_SIZE);
    [[maybe_unused]] void *secondRecord = stack.Acquire (SECOND_RECORD_SIZE);
    void *thirdRecord = stack.Acquire (THIRD_RECORD_SIZE);

    stack.Release (cachedHead);
    CHECK_EQUAL (stack.Head (), cachedHead);
    CHECK_EQUAL (stack.Acquire (FIRST_RECORD_SIZE + SECOND_RECORD_SIZE), firstRecord);
    CHECK_EQUAL (stack.Acquire (THIRD_RECORD_SIZE), thirdRecord);
}

TEST_CASE (FreeSize)
{
    constexpr size_t STACK_SIZE = 1024u;
    Emergence::Memory::Stack stack {STACK_SIZE};
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
    Emergence::Memory::Stack stack {1024u};
    [[maybe_unused]] void *firstRecord = stack.Acquire (32u);
    const void *cachedHead = stack.Head ();

    Emergence::Memory::Stack movedStack {std::move (stack)};
    CHECK_EQUAL (movedStack.Head (), cachedHead);

    // Acquire to check usability.
    [[maybe_unused]] void *secondRecord = movedStack.Acquire (128u);
}

TEST_CASE (MoveAssign)
{
    Emergence::Memory::Stack stack {1024u};
    Emergence::Memory::Stack anotherStack {1024u};

    [[maybe_unused]] void *firstRecord = stack.Acquire (32u);
    const void *cachedHead = stack.Head ();

    anotherStack = std::move (stack);
    CHECK_EQUAL (anotherStack.Head (), cachedHead);

    // Acquire to check usability.
    [[maybe_unused]] void *secondRecord = anotherStack.Acquire (128u);
}

END_SUITE
