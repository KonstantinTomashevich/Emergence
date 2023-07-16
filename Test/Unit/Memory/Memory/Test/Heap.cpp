#define _CRT_SECURE_NO_WARNINGS

#include <cstring>
#include <vector>

#include <Memory/Heap.hpp>
#include <Memory/Test/Helpers.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory::Literals;
using namespace Emergence::Memory::Test;

BEGIN_SUITE (Heap)

TEST_CASE (AcquireNoOverlap)
{
    static const char *firstString = "Hello, world!";
    static const char *secondString = "Hello, world again!";

    Emergence::Memory::Heap heap {GetUniqueAllocationGroup ()};
    void *first = heap.Acquire (strlen (firstString) + 1u, sizeof (std::uintptr_t));
    void *second = heap.Acquire (strlen (secondString) + 1u, sizeof (std::uintptr_t));

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
    Emergence::Memory::Heap heap {GetUniqueAllocationGroup ()};
    void *record = heap.Acquire (SIZE, sizeof (std::uintptr_t));

    CHECK_EQUAL (reinterpret_cast<std::uintptr_t> (record) % sizeof (std::uintptr_t), 0u);
    heap.Release (record, SIZE);
}

TEST_CASE (Resize)
{
    static const char *string = "Hello, world!";
    static const std::size_t size = strlen (string) + 1u;
    constexpr std::size_t NEW_SIZE = 1024u;

    Emergence::Memory::Heap heap {GetUniqueAllocationGroup ()};
    void *record = heap.Acquire (size, sizeof (std::uintptr_t));

    strcpy (reinterpret_cast<char *> (record), string);
    CHECK (strcmp (reinterpret_cast<char *> (record), string) == 0);

    record = heap.Resize (record, sizeof (std::uintptr_t), size, NEW_SIZE);
    CHECK (strcmp (reinterpret_cast<char *> (record), string) == 0);
    heap.Release (record, NEW_SIZE);
}

TEST_CASE (HeapSTD)
{
    std::vector<std::uintptr_t, Emergence::Memory::HeapSTD<std::uintptr_t>> vector {
        Emergence::Memory::HeapSTD<std::uintptr_t> {GetUniqueAllocationGroup ()}};

    vector.reserve (8u);
    for (std::size_t index = 0u; index < 1024u; ++index)
    {
        vector.emplace_back (index);
    }

    for (std::size_t index = 0u; index < 1024u; ++index)
    {
        CHECK_EQUAL (vector[index], index);
    }
}

TEST_CASE (Profiling)
{
    Emergence::Memory::Profiler::AllocationGroup group = GetUniqueAllocationGroup ();
    Emergence::Memory::Heap heap {group};
    CHECK_EQUAL (group.GetTotal (), 0u);
    CHECK_EQUAL (group.GetReserved (), 0u);
    CHECK_EQUAL (group.GetAcquired (), 0u);

    constexpr std::size_t FIRST_OBJECT_SIZE = 32u;
    constexpr std::size_t SECOND_OBJECT_SIZE = 128u;

    void *first = heap.Acquire (FIRST_OBJECT_SIZE, sizeof (std::uintptr_t));
    CHECK_EQUAL (group.GetTotal (), FIRST_OBJECT_SIZE);
    CHECK_EQUAL (group.GetReserved (), 0u);
    CHECK_EQUAL (group.GetAcquired (), FIRST_OBJECT_SIZE);

    void *second = heap.Acquire (SECOND_OBJECT_SIZE, sizeof (std::uintptr_t));
    CHECK_EQUAL (group.GetTotal (), FIRST_OBJECT_SIZE + SECOND_OBJECT_SIZE);
    CHECK_EQUAL (group.GetReserved (), 0u);
    CHECK_EQUAL (group.GetAcquired (), FIRST_OBJECT_SIZE + SECOND_OBJECT_SIZE);

    heap.Release (first, FIRST_OBJECT_SIZE);
    CHECK_EQUAL (group.GetTotal (), SECOND_OBJECT_SIZE);
    CHECK_EQUAL (group.GetReserved (), 0u);
    CHECK_EQUAL (group.GetAcquired (), SECOND_OBJECT_SIZE);

    heap.Release (second, SECOND_OBJECT_SIZE);
    CHECK_EQUAL (group.GetTotal (), 0u);
    CHECK_EQUAL (group.GetReserved (), 0u);
    CHECK_EQUAL (group.GetAcquired (), 0u);
}

END_SUITE
