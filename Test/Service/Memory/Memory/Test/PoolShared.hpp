#include <Container/Vector.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>
#include <Memory/Test/Helpers.hpp>
#include <Memory/UniqueString.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Test::Pool
{
using namespace Emergence::Memory::Literals;

/// \brief Memory profiler reports internal page data as part of total data usage.
constexpr size_t RESERVED_FOR_PAGE_INTERNALS = sizeof (uintptr_t);

struct TestItem
{
    uint64_t integer;
    float floating;
    bool flag;
};

template <typename Pool>
struct FullPoolContext
{
    static constexpr std::size_t PAGE_CAPACITY = 8u;
    static constexpr std::size_t PAGES_TO_FILL = 2u;

    FullPoolContext<Pool> ()
    {
        for (std::size_t index = 0u; index < PAGES_TO_FILL * PAGE_CAPACITY; ++index)
        {
            items.emplace_back (pool.Acquire ());
        }

        CHECK_EQUAL (pool.GetAllocationGroup ().GetTotal (),
                     PAGES_TO_FILL * (PAGE_CAPACITY * sizeof (TestItem) + RESERVED_FOR_PAGE_INTERNALS));
    }

    Pool pool {GetUniqueAllocationGroup (), sizeof (TestItem), PAGE_CAPACITY};
    Container::Vector<void *> items;
};

template <typename Pool>
void AcquireNotNull ()
{
    Pool pool {GetUniqueAllocationGroup (), sizeof (TestItem)};
    CHECK (pool.Acquire ());
}

template <typename Pool>
void MultipleAcquiresDoNotOverlap ()
{
    Pool pool {GetUniqueAllocationGroup (), sizeof (TestItem)};
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

template <typename Pool>
void MemoryReused ()
{
    Pool pool {GetUniqueAllocationGroup (), sizeof (TestItem)};
    void *item = pool.Acquire ();
    pool.Release (item);

    void *anotherItem = pool.Acquire ();
    CHECK_EQUAL (item, anotherItem);
}

template <typename Pool>
void Clear ()
{
    FullPoolContext<Pool> context;
    context.pool.Clear ();
    CHECK_EQUAL (context.pool.GetAllocationGroup ().GetTotal (), 0u);

    // Acquire one item to ensure that pool is in working state.
    CHECK (context.pool.Acquire ());
}

template <typename Pool>
void Move ()
{
    FullPoolContext<Pool> context;
    Pool newPool (std::move (context.pool));

    CHECK_EQUAL (context.pool.GetAllocationGroup ().GetTotal (), 0u);
    CHECK_EQUAL (newPool.GetAllocationGroup ().GetTotal (),
                 FullPoolContext<Pool>::PAGES_TO_FILL *
                     (FullPoolContext<Pool>::PAGE_CAPACITY * sizeof (TestItem) + RESERVED_FOR_PAGE_INTERNALS));

    // Acquire one item from each pool to ensure that they are in working state.
    CHECK (context.pool.Acquire ());
    CHECK (newPool.Acquire ());
}

template <typename Pool>
void MoveAssign ()
{
    FullPoolContext<Pool> firstContext;
    FullPoolContext<Pool> secondContext;
    secondContext.pool = std::move (firstContext.pool);

    CHECK_EQUAL (firstContext.pool.GetAllocationGroup ().GetTotal (), 0u);
    CHECK_EQUAL (secondContext.pool.GetAllocationGroup ().GetTotal (),
                 FullPoolContext<Pool>::PAGES_TO_FILL *
                     (FullPoolContext<Pool>::PAGE_CAPACITY * sizeof (TestItem) + RESERVED_FOR_PAGE_INTERNALS));

    // Acquire one item from each pool to ensure that they are in working state.
    CHECK (firstContext.pool.Acquire ());
    CHECK (secondContext.pool.Acquire ());
}

template <typename Pool>
void Profiling ()
{
    Profiler::AllocationGroup group = GetUniqueAllocationGroup ();
    Pool pool {group, sizeof (TestItem), 2u};
    CHECK_EQUAL (group.GetReserved (), 0u);
    CHECK_EQUAL (group.GetAcquired (), 0u);
    CHECK_EQUAL (group.GetTotal (), 0u);

    [[maybe_unused]] void *first = pool.Acquire ();
    CHECK_EQUAL (group.GetReserved (), sizeof (TestItem));
    CHECK_EQUAL (group.GetAcquired (), RESERVED_FOR_PAGE_INTERNALS + sizeof (TestItem));
    CHECK_EQUAL (group.GetTotal (), RESERVED_FOR_PAGE_INTERNALS + sizeof (TestItem) * 2u);

    [[maybe_unused]] void *second = pool.Acquire ();
    CHECK_EQUAL (group.GetReserved (), 0u);
    CHECK_EQUAL (group.GetAcquired (), RESERVED_FOR_PAGE_INTERNALS + sizeof (TestItem) * 2u);
    CHECK_EQUAL (group.GetTotal (), RESERVED_FOR_PAGE_INTERNALS + sizeof (TestItem) * 2u);

    void *third = pool.Acquire ();
    CHECK_EQUAL (group.GetReserved (), sizeof (TestItem));
    CHECK_EQUAL (group.GetAcquired (), 2u * RESERVED_FOR_PAGE_INTERNALS + 3u * sizeof (TestItem));
    CHECK_EQUAL (group.GetTotal (), 2u * (RESERVED_FOR_PAGE_INTERNALS + sizeof (TestItem) * 2u));

    pool.Release (third);
    CHECK_EQUAL (group.GetReserved (), 2u * sizeof (TestItem));
    CHECK_EQUAL (group.GetAcquired (), 2u * RESERVED_FOR_PAGE_INTERNALS + 2u * sizeof (TestItem));
    CHECK_EQUAL (group.GetTotal (), 2u * (RESERVED_FOR_PAGE_INTERNALS + sizeof (TestItem) * 2u));

    pool.Clear ();
    CHECK_EQUAL (group.GetReserved (), 0u);
    CHECK_EQUAL (group.GetAcquired (), 0u);
    CHECK_EQUAL (group.GetTotal (), 0u);
}
} // namespace Emergence::Memory::Test::Pool

#define SHARED_POOL_TEST(ImplementationClass, TestName)                                                                \
    TEST_CASE (TestName)                                                                                               \
    {                                                                                                                  \
        Emergence::Memory::Test::Pool::TestName<ImplementationClass> ();                                               \
    }

#define ALL_SHARED_POOL_TESTS(ImplementationClass)                                                                     \
    SHARED_POOL_TEST (ImplementationClass, AcquireNotNull)                                                             \
    SHARED_POOL_TEST (ImplementationClass, MultipleAcquiresDoNotOverlap)                                               \
    SHARED_POOL_TEST (ImplementationClass, MemoryReused)                                                               \
    SHARED_POOL_TEST (ImplementationClass, Clear)                                                                      \
    SHARED_POOL_TEST (ImplementationClass, Move)                                                                       \
    SHARED_POOL_TEST (ImplementationClass, MoveAssign)                                                                 \
    SHARED_POOL_TEST (ImplementationClass, Profiling)
