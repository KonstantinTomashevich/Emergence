#include <Memory/OrderedPool.hpp>
#include <Memory/Test/OrderedPool.hpp>
#include <Memory/Test/PoolShared.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Test
{
bool OrderedPoolTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Memory::Test

using namespace Emergence::Memory::Literals;
using namespace Emergence::Memory::Test::Pool;

BEGIN_SUITE (OrderedPool)

ALL_SHARED_POOL_TESTS (Emergence::Memory::OrderedPool)

using PoolContext = FullPoolContext<Emergence::Memory::OrderedPool>;

TEST_CASE (SuccessfullShrink)
{
    static_assert (PoolContext::PAGES_TO_FILL >= 2u);
    PoolContext context;

    // Release first half of the first page.
    for (std::size_t index = 0u; index < PoolContext::PAGE_CAPACITY / 2u; ++index)
    {
        context.pool.Release (context.items[index]);
    }

    // Release whole second page.
    for (std::size_t index = 0u; index < PoolContext::PAGE_CAPACITY; ++index)
    {
        context.pool.Release (context.items[PoolContext::PAGE_CAPACITY + index]);
    }

    context.pool.Shrink ();
    CHECK_EQUAL (context.pool.GetAllocationGroup ().GetTotal (),
                 (PoolContext::PAGES_TO_FILL - 1u) * PoolContext::PAGE_CAPACITY * sizeof (TestItem));

    // Acquire one item to ensure that pool is in working state.
    CHECK (context.pool.Acquire ());
}

TEST_CASE (UnsuccessfullShrink)
{
    PoolContext context;

    for (std::size_t index = 0u; index < PoolContext::PAGE_CAPACITY * PoolContext::PAGES_TO_FILL; index += 2u)
    {
        context.pool.Release (context.items[index]);
    }

    context.pool.Shrink ();
    CHECK_EQUAL (context.pool.GetAllocationGroup ().GetTotal (),
                 PoolContext::PAGES_TO_FILL * PoolContext::PAGE_CAPACITY * sizeof (TestItem));

    // Acquire one item to ensure that pool is in working state.
    CHECK (context.pool.Acquire ());
}

TEST_CASE (Clear)
{
    PoolContext context;
    context.pool.Clear ();
    CHECK_EQUAL (context.pool.GetAllocationGroup ().GetTotal (), 0u);

    // Acquire one item to ensure that pool is in working state.
    CHECK (context.pool.Acquire ());
}

TEST_CASE (Move)
{
    PoolContext context;
    Emergence::Memory::OrderedPool newPool (std::move (context.pool));

    CHECK_EQUAL (context.pool.GetAllocationGroup ().GetTotal (), 0u);
    CHECK_EQUAL (newPool.GetAllocationGroup ().GetTotal (),
                 PoolContext::PAGES_TO_FILL * PoolContext::PAGE_CAPACITY * sizeof (TestItem));

    // Acquire one item from each pool to ensure that they are in working state.
    CHECK (context.pool.Acquire ());
    CHECK (newPool.Acquire ());
}

TEST_CASE (IterateEmpty)
{
    Emergence::Memory::OrderedPool pool {Emergence::Memory::Profiler::AllocationGroup {"Test"_us}, sizeof (TestItem)};
    CHECK (pool.BeginAcquired () == pool.EndAcquired ());
}

TEST_CASE (IterationFirstItem)
{
    Emergence::Memory::OrderedPool pool {Emergence::Memory::Profiler::AllocationGroup {"Test"_us}, sizeof (TestItem)};
    void *first = pool.Acquire ();
    void *second = pool.Acquire ();

    CHECK_EQUAL (first, *pool.BeginAcquired ());
    pool.Release (first);
    CHECK_EQUAL (second, *pool.BeginAcquired ());

    first = pool.Acquire ();
    pool.Release (second);
    CHECK_EQUAL (first, *pool.BeginAcquired ());
}

void CheckPoolIteration (Emergence::Memory::OrderedPool &_pool,
                         const Emergence::Container::Vector<void *> &_expectedItems)
{
    Emergence::Container::Vector<void *> itemsFromIteration;
    for (void *item : _pool)
    {
        itemsFromIteration.emplace_back (item);
    }

    auto checkPoolItemVectorsEquality =
        [] (const Emergence::Container::Vector<void *> &_first, const Emergence::Container::Vector<void *> &_second)
    {
        CHECK_EQUAL (_first.size (), _second.size ());

        // Check that there is no duplicates in first vector.
        for (auto iterator = _first.begin (); iterator != _first.end (); ++iterator)
        {
            auto searchIterator = std::find (std::next (iterator), _first.end (), *iterator);
            CHECK_WITH_MESSAGE (searchIterator == _first.end (), "Searching for duplicate of item ", *iterator, ".");
        }

        // Order of expected and received items can be different,
        // because new page address may be lower than address of any previous page.
        for (void *item : _first)
        {
            auto iterator = std::find (_second.begin (), _second.end (), item);
            CHECK_WITH_MESSAGE (iterator != _second.end (), "Searching for item ", item, ".");
        }
    };

    checkPoolItemVectorsEquality (_expectedItems, itemsFromIteration);
    itemsFromIteration.clear ();

    for (const void *item : const_cast<const Emergence::Memory::OrderedPool &> (_pool))
    {
        itemsFromIteration.emplace_back (const_cast<void *> (item));
    }

    checkPoolItemVectorsEquality (_expectedItems, itemsFromIteration);
}

TEST_CASE (IterateFull)
{
    PoolContext context;
    CheckPoolIteration (context.pool, context.items);
}

TEST_CASE (IterateFullWithGaps)
{
    PoolContext context;
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

TEST_CASE (IteratorConstructionAndAssignment)
{
    auto test = [] (const auto _begin)
    {
        auto next = _begin;
        auto previous = next++;

        CHECK (_begin != next);
        CHECK (previous != next);
        CHECK (previous == _begin);

        CHECK_NOT_EQUAL (*_begin, *next);
        CHECK_NOT_EQUAL (*previous, *next);
        CHECK_EQUAL (*previous, *_begin);

        next = _begin;
        CHECK (_begin == next);
        CHECK (previous == next);
        CHECK_EQUAL (*next, *_begin);
    };

    PoolContext context;
    // Test both mutable and const iterators.
    test (context.pool.BeginAcquired ());
    test (const_cast<const Emergence::Memory::OrderedPool &> (context.pool).BeginAcquired ());
}

END_SUITE
