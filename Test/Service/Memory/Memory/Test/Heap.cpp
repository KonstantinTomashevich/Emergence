#define _CRT_SECURE_NO_WARNINGS

#include <cstring>
#include <vector>

#include <Memory/Heap.hpp>
#include <Memory/Test/Heap.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Test
{
bool HeapTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Memory::Test

using namespace Emergence::Memory::Literals;

static Emergence::Memory::Profiler::AllocationGroup testAllocationGroup {"Test"_us};

BEGIN_SUITE (Heap)

TEST_CASE (AcquireNoOverlap)
{
    static const char *firstString = "Hello, world!";
    static const char *secondString = "Hello, world again!";

    Emergence::Memory::Heap heap {testAllocationGroup};
    void *first = heap.Acquire (strlen (firstString) + 1u);
    void *second = heap.Acquire (strlen (secondString) + 1u);

    strcpy (reinterpret_cast<char *> (first), firstString);
    strcpy (reinterpret_cast<char *> (second), secondString);

    CHECK (strcmp (reinterpret_cast<char *> (first), firstString) == 0);
    CHECK (strcmp (reinterpret_cast<char *> (second), secondString) == 0);

    heap.Release (first, strlen (firstString) + 1u);
    heap.Release (second, strlen (secondString) + 1u);
}

TEST_CASE (AcquireAlignment)
{
    constexpr std::size_t SIZE = 128u;
    Emergence::Memory::Heap heap {testAllocationGroup};
    void *record = heap.Acquire (SIZE);

    CHECK_EQUAL (reinterpret_cast<uintptr_t> (record) % sizeof (uintptr_t), 0u);
    heap.Release (record, SIZE);
}

TEST_CASE (Resize)
{
    static const char *string = "Hello, world!";
    static const std::size_t SIZE = strlen (string) + 1u;
    constexpr std::size_t NEW_SIZE = 1024u;

    Emergence::Memory::Heap heap {testAllocationGroup};
    void *record = heap.Acquire (SIZE);

    strcpy (reinterpret_cast<char *> (record), string);
    CHECK (strcmp (reinterpret_cast<char *> (record), string) == 0);

    record = heap.Resize (record, SIZE, NEW_SIZE);
    CHECK (strcmp (reinterpret_cast<char *> (record), string) == 0);
    heap.Release (record, NEW_SIZE);
}

TEST_CASE (HeapSTD)
{
    std::vector<uintptr_t, Emergence::Memory::HeapSTD<uintptr_t>> vector {
        Emergence::Memory::HeapSTD<uintptr_t> {testAllocationGroup}};

    vector.reserve (8u);
    for (size_t index = 0u; index < 1024; ++index)
    {
        vector.emplace_back (index);
    }

    for (size_t index = 0u; index < 1024; ++index)
    {
        CHECK_EQUAL (vector[index], index);
    }
}

END_SUITE
