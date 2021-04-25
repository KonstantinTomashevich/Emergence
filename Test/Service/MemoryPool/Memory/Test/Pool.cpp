#include <boost/test/unit_test.hpp>

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

BOOST_AUTO_TEST_SUITE_END ()