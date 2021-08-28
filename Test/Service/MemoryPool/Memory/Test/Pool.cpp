#include <vector>

#include <Memory/Pool.hpp>
#include <Memory/Test/Pool.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Test
{
bool PoolTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Memory::Test

BEGIN_SUITE (Pool)

struct TestItem
{
    uint64_t integer;
    float floating;
    bool flag;
};

TEST_CASE (AcquireNotNull)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    CHECK (pool.Acquire ());
}

TEST_CASE (MultipleAcquiresDoNotOverlap)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    auto *first = static_cast<TestItem *> (pool.Acquire ());
    auto *second = static_cast<TestItem *> (pool.Acquire ());

    TestItem firstValue = {12, 341.55f, false};
    TestItem secondValue = {59, 947.11f, true};

    *first = firstValue;
    *second = secondValue;

    CHECK_EQUAL (firstValue.integer, first->integer);
    CHECK_EQUAL (firstValue.floating, first->floating);
    CHECK_EQUAL (firstValue.flag, first->flag);

    CHECK_EQUAL (secondValue.integer, second->integer);
    CHECK_EQUAL (secondValue.floating, second->floating);
    CHECK_EQUAL (secondValue.flag, second->flag);
}

TEST_CASE (MemoryReused)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    void *item = pool.Acquire ();
    pool.Release (item);

    void *anotherItem = pool.Acquire ();
    CHECK_EQUAL (item, anotherItem);
}

void CheckPoolItemVectorsEquality (const std::vector<void *> &_first, const std::vector<void *> &_second)
{
    CHECK_EQUAL (_first.size (), _second.size ());

    // Items order is not guaranteed.
    for (void *item : _first)
    {
        auto iterator = std::find (_second.begin (), _second.end (), item);
        CHECK_WITH_MESSAGE (iterator != _second.end (), "Searching for item ", item, ".");
    }
}

void CheckPoolIteration (Emergence::Memory::Pool &_pool, const std::vector<void *> &_expectedItems)
{
    std::vector<void *> itemsFromIteration;
    for (void *item : _pool)
    {
        itemsFromIteration.emplace_back (item);
    }

    CheckPoolItemVectorsEquality (_expectedItems, itemsFromIteration);
    itemsFromIteration.clear ();

    for (const void *item : const_cast<const Emergence::Memory::Pool &> (_pool))
    {
        itemsFromIteration.emplace_back (const_cast<void *> (item));
    }

    CheckPoolItemVectorsEquality (_expectedItems, itemsFromIteration);
}

struct FullPoolContext
{
    static constexpr std::size_t PAGE_CAPACITY = 8u;
    static constexpr std::size_t PAGES_TO_FILL = 2u;

    FullPoolContext ()
    {
        for (std::size_t index = 0u; index < PAGES_TO_FILL * PAGE_CAPACITY; ++index)
        {
            items.emplace_back (pool.Acquire ());
        }

        CHECK_EQUAL (pool.GetAllocatedSpace (), PAGES_TO_FILL * PAGE_CAPACITY * sizeof (TestItem));
    }

    Emergence::Memory::Pool pool {sizeof (TestItem), PAGE_CAPACITY};
    std::vector<void *> items;
};

TEST_CASE (SuccessfullShrink)
{
    static_assert (FullPoolContext::PAGES_TO_FILL >= 2u);
    FullPoolContext context;

    for (std::size_t index = 0u; index < FullPoolContext::PAGE_CAPACITY + FullPoolContext::PAGE_CAPACITY / 2u; ++index)
    {
        context.pool.Release (context.items[index]);
    }

    context.pool.Shrink ();
    CHECK_EQUAL (context.pool.GetAllocatedSpace (),
                 (FullPoolContext::PAGES_TO_FILL - 1u) * FullPoolContext::PAGE_CAPACITY * sizeof (TestItem));

    // Acquire one item to ensure that pool is in working state.
    CHECK (context.pool.Acquire ());
}

TEST_CASE (UnsuccessfullShrink)
{
    FullPoolContext context;

    for (std::size_t index = 0u; index < FullPoolContext::PAGE_CAPACITY * FullPoolContext::PAGES_TO_FILL; index += 2u)
    {
        context.pool.Release (context.items[index]);
    }

    context.pool.Shrink ();
    CHECK_EQUAL (context.pool.GetAllocatedSpace (),
                 FullPoolContext::PAGES_TO_FILL * FullPoolContext::PAGE_CAPACITY * sizeof (TestItem));

    // Acquire one item to ensure that pool is in working state.
    CHECK (context.pool.Acquire ());
}

TEST_CASE (Clear)
{
    FullPoolContext context;
    context.pool.Clear ();
    CHECK_EQUAL (context.pool.GetAllocatedSpace (), 0u);

    // Acquire one item to ensure that pool is in working state.
    CHECK (context.pool.Acquire ());
}

TEST_CASE (Move)
{
    FullPoolContext context;
    Emergence::Memory::Pool newPool (std::move (context.pool));

    CHECK_EQUAL (context.pool.GetAllocatedSpace (), 0u);
    CHECK_EQUAL (newPool.GetAllocatedSpace (),
                 FullPoolContext::PAGES_TO_FILL * FullPoolContext::PAGE_CAPACITY * sizeof (TestItem));

    // Acquire one item from each pool to ensure that they are in working state.
    CHECK (context.pool.Acquire ());
    CHECK (newPool.Acquire ());
}

TEST_CASE (IterateEmpty)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    CHECK (pool.BeginAcquired () == pool.EndAcquired ());
}

TEST_CASE (IterationFirstItem)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    void *first = pool.Acquire ();
    void *second = pool.Acquire ();

    CHECK_EQUAL (first, *pool.BeginAcquired ());
    pool.Release (first);
    CHECK_EQUAL (second, *pool.BeginAcquired ());

    first = pool.Acquire ();
    pool.Release (second);
    CHECK_EQUAL (first, *pool.BeginAcquired ());
}

TEST_CASE (IterateFull)
{
    FullPoolContext context;
    CheckPoolIteration (context.pool, context.items);
}

TEST_CASE (IterateFullWithGaps)
{
    FullPoolContext context;
    auto iterator = context.items.begin ();
    std::size_t index = 0u;

    while (iterator != context.items.end ())
    {
        if (index % 2u)
        {
            context.pool.Release (*iterator);
            iterator = context.items.erase (iterator);
        }
        else
        {
            ++iterator;
        }

        ++index;
    }

    CheckPoolIteration (context.pool, context.items);
}

TEST_CASE (ReleaseDoesNotInvalidateIterator)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    std::vector<void *> items;

    // Must be greater than 1. If it's greater than 1, value should not matter.
    constexpr std::size_t itemsToAcquire = 5u;

    for (std::size_t index = 0u; index < itemsToAcquire; ++index)
    {
        items.emplace_back (pool.Acquire ());
    }

    std::vector<void *> itemsFromIteration;
    auto iterator = pool.BeginAcquired ();
    auto end = pool.EndAcquired ();

    // Release first item, that is not pointed by iterator.
    for (auto iteratorToDelete = items.begin (); iteratorToDelete != items.end (); ++iteratorToDelete)
    {
        if (*iteratorToDelete != *iterator)
        {
            pool.Release (*iteratorToDelete);
            items.erase (iteratorToDelete);
            break;
        }
    }

    while (iterator != end)
    {
        itemsFromIteration.emplace_back (*iterator);
        ++iterator;
    }

    CheckPoolItemVectorsEquality (items, itemsFromIteration);
}

TEST_CASE (CursorConstructionAndAssignment)
{
    auto Test = [] (const auto begin)
    {
        auto next = begin;
        auto previous = next++;

        CHECK (begin != next);
        CHECK (previous != next);
        CHECK (previous == begin);

        CHECK_NOT_EQUAL (*begin, *next);
        CHECK_NOT_EQUAL (*previous, *next);
        CHECK_EQUAL (*previous, *begin);

        next = begin;
        CHECK (begin == next);
        CHECK (previous == next);
        CHECK_EQUAL (*next, *begin);
    };

    FullPoolContext context;
    // Test both mutable and const iterators.
    Test (context.pool.BeginAcquired ());
    Test (const_cast<const Emergence::Memory::Pool &> (context.pool).BeginAcquired ());
}

END_SUITE
