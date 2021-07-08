#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include <Memory/Pool.hpp>
#include <Memory/Test/Pool.hpp>

namespace Emergence::Memory::Test
{
bool PoolTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Memory::Test

BOOST_AUTO_TEST_SUITE (Pool)

struct TestItem
{
    uint64_t integer;
    float floating;
    bool flag;
};

BOOST_AUTO_TEST_CASE (AcquireNotNull)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    BOOST_CHECK (pool.Acquire ());
}

BOOST_AUTO_TEST_CASE (MultipleAcquiresDoNotOverlap)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    auto *first = static_cast <TestItem *> (pool.Acquire ());
    auto *second = static_cast <TestItem *> (pool.Acquire ());

    TestItem firstValue = {12, 341.55f, false};
    TestItem secondValue = {59, 947.11f, true};

    *first = firstValue;
    *second = secondValue;

    BOOST_CHECK_EQUAL (firstValue.integer, first->integer);
    BOOST_CHECK_EQUAL (firstValue.floating, first->floating);
    BOOST_CHECK_EQUAL (firstValue.flag, first->flag);

    BOOST_CHECK_EQUAL (secondValue.integer, second->integer);
    BOOST_CHECK_EQUAL (secondValue.floating, second->floating);
    BOOST_CHECK_EQUAL (secondValue.flag, second->flag);
}

BOOST_AUTO_TEST_CASE (MemoryReused)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    void *item = pool.Acquire ();
    pool.Release (item);

    void *anotherItem = pool.Acquire ();
    BOOST_CHECK_EQUAL (item, anotherItem);
}

void CheckPoolItemVectorsEquality (const std::vector <void *> &_first, const std::vector <void *> &_second)
{
    BOOST_CHECK_EQUAL (_first.size (), _second.size ());

    // Items order is not guaranteed.
    for (void *item : _first)
    {
        auto iterator = std::find (_second.begin (), _second.end (), item);
        BOOST_CHECK_MESSAGE (iterator != _second.end (),
                             boost::format ("Searching for item %1%.") % item);
    }
}

void CheckPoolIteration (Emergence::Memory::Pool &_pool, const std::vector <void *> &_expectedItems)
{
    std::vector <void *> itemsFromIteration;
    for (void *item : _pool)
    {
        itemsFromIteration.emplace_back (item);
    }

    CheckPoolItemVectorsEquality (_expectedItems, itemsFromIteration);
    itemsFromIteration.clear ();

    for (const void *item : const_cast <const Emergence::Memory::Pool &> (_pool))
    {
        itemsFromIteration.emplace_back (const_cast <void *> (item));
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

        BOOST_CHECK_EQUAL (pool.GetAllocatedSpace (), PAGES_TO_FILL * PAGE_CAPACITY * sizeof (TestItem));
    }

    Emergence::Memory::Pool pool {sizeof (TestItem), PAGE_CAPACITY};
    std::vector <void *> items;
};

BOOST_AUTO_TEST_CASE (SuccessfullShrink)
{
    static_assert (FullPoolContext::PAGES_TO_FILL >= 2u);
    FullPoolContext context;

    for (std::size_t index = 0u; index < FullPoolContext::PAGE_CAPACITY + FullPoolContext::PAGE_CAPACITY / 2u; ++index)
    {
        context.pool.Release (context.items[index]);
    }

    context.pool.Shrink ();
    BOOST_CHECK_EQUAL (context.pool.GetAllocatedSpace (),
                       (FullPoolContext::PAGES_TO_FILL - 1u) * FullPoolContext::PAGE_CAPACITY * sizeof (TestItem));

    // Acquire one item to ensure that pool is in working state.
    BOOST_CHECK (context.pool.Acquire ());
}

BOOST_AUTO_TEST_CASE (UnsuccessfullShrink)
{
    FullPoolContext context;

    for (std::size_t index = 0u; index < FullPoolContext::PAGE_CAPACITY * FullPoolContext::PAGES_TO_FILL; index += 2u)
    {
        context.pool.Release (context.items[index]);
    }

    context.pool.Shrink ();
    BOOST_CHECK_EQUAL (context.pool.GetAllocatedSpace (),
                       FullPoolContext::PAGES_TO_FILL * FullPoolContext::PAGE_CAPACITY * sizeof (TestItem));

    // Acquire one item to ensure that pool is in working state.
    BOOST_CHECK (context.pool.Acquire ());
}

BOOST_AUTO_TEST_CASE (Clear)
{
    FullPoolContext context;
    context.pool.Clear ();
    BOOST_CHECK_EQUAL (context.pool.GetAllocatedSpace (), 0u);

    // Acquire one item to ensure that pool is in working state.
    BOOST_CHECK (context.pool.Acquire ());
}

BOOST_AUTO_TEST_CASE (Move)
{
    FullPoolContext context;
    Emergence::Memory::Pool newPool (std::move (context.pool));

    BOOST_CHECK_EQUAL (context.pool.GetAllocatedSpace (), 0u);
    BOOST_CHECK_EQUAL (newPool.GetAllocatedSpace (),
                       FullPoolContext::PAGES_TO_FILL * FullPoolContext::PAGE_CAPACITY * sizeof (TestItem));

    // Acquire one item from each pool to ensure that they are in working state.
    BOOST_CHECK (context.pool.Acquire ());
    BOOST_CHECK (newPool.Acquire ());
}

BOOST_AUTO_TEST_CASE (IterateEmpty)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    BOOST_CHECK (pool.BeginAcquired () == pool.EndAcquired ());
}

BOOST_AUTO_TEST_CASE (IterationFirstItem)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    void *first = pool.Acquire ();
    void *second = pool.Acquire ();

    BOOST_CHECK_EQUAL (first, *pool.BeginAcquired ());
    pool.Release (first);
    BOOST_CHECK_EQUAL (second, *pool.BeginAcquired ());

    first = pool.Acquire ();
    pool.Release (second);
    BOOST_CHECK_EQUAL (first, *pool.BeginAcquired ());
}

BOOST_AUTO_TEST_CASE (IterateFull)
{
    FullPoolContext context;
    CheckPoolIteration (context.pool, context.items);
}

BOOST_AUTO_TEST_CASE (IterateFullWithGaps)
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

BOOST_AUTO_TEST_CASE (ReleaseDoesNotInvalidateIterator)
{
    Emergence::Memory::Pool pool {sizeof (TestItem)};
    std::vector <void *> items;

    // Must be greater than 1. If it's greater than 1, value should not matter.
    constexpr std::size_t itemsToAcquire = 5u;

    for (std::size_t index = 0u; index < itemsToAcquire; ++index)
    {
        items.emplace_back (pool.Acquire ());
    }

    std::vector <void *> itemsFromIteration;
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

BOOST_AUTO_TEST_SUITE_END ()