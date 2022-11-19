#include <API/Common/Implementation/Iterator.hpp>

#include <Assert/Assert.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/Profiler/Original/AllocationGroup.hpp>
#include <Memory/Profiler/Original/ProfilingLock.hpp>

namespace Emergence::Memory::Profiler
{
AllocationGroup::PlacedOnStack::~PlacedOnStack ()
{
    Original::AllocationGroupStack::Get ().Pop ();
}

AllocationGroup::PlacedOnStack::PlacedOnStack (void *_groupHandle) noexcept
{
    Original::AllocationGroupStack::Get ().Push (static_cast<Original::AllocationGroup *> (_groupHandle));
}

using Iterator = AllocationGroup::Iterator;

using IteratorImplementation = Original::AllocationGroup::Iterator;

AllocationGroup AllocationGroup::Iterator::operator* () const noexcept
{
    return AllocationGroup (*block_cast<IteratorImplementation> (data));
}

EMERGENCE_BIND_FORWARD_ITERATOR_OPERATIONS_IMPLEMENTATION (Iterator, IteratorImplementation)

AllocationGroup AllocationGroup::Root () noexcept
{
    return AllocationGroup (Original::AllocationGroup::Root ());
}

AllocationGroup AllocationGroup::Top () noexcept
{
    return AllocationGroup (Original::AllocationGroupStack::Get ().Top ());
}

AllocationGroup::AllocationGroup () noexcept
    : handle (nullptr)
{
}

AllocationGroup::AllocationGroup (UniqueString _id) noexcept
{
    Original::ProfilingLock lock;
    handle = Original::AllocationGroup::Request (_id, lock);
}

AllocationGroup::AllocationGroup (const AllocationGroup &_parent, UniqueString _id) noexcept
{
    Original::ProfilingLock lock;
    handle = Original::AllocationGroup::Request (static_cast<Original::AllocationGroup *> (_parent.handle), _id, lock);
}

AllocationGroup::AllocationGroup (const AllocationGroup &_other) noexcept = default;

AllocationGroup::AllocationGroup (AllocationGroup &&_other) noexcept
    : handle (_other.handle)
{
    _other.handle = nullptr;
}

AllocationGroup::~AllocationGroup () noexcept = default;

AllocationGroup::PlacedOnStack AllocationGroup::PlaceOnTop () const noexcept
{
    EMERGENCE_ASSERT (handle);
    return AllocationGroup::PlacedOnStack (handle);
}

void AllocationGroup::Allocate (size_t _bytesCount) noexcept
{
    if (handle)
    {
        Original::ProfilingLock lock;
        static_cast<Original::AllocationGroup *> (handle)->Allocate (_bytesCount, lock);
    }
}

void AllocationGroup::Acquire (size_t _bytesCount) noexcept
{
    if (handle)
    {
        Original::ProfilingLock lock;
        static_cast<Original::AllocationGroup *> (handle)->Acquire (_bytesCount, lock);
    }
}

void AllocationGroup::Release (size_t _bytesCount) noexcept
{
    if (handle)
    {
        Original::ProfilingLock lock;
        static_cast<Original::AllocationGroup *> (handle)->Release (_bytesCount, lock);
    }
}

void AllocationGroup::Free (size_t _bytesCount) noexcept
{
    if (handle)
    {
        Original::ProfilingLock lock;
        static_cast<Original::AllocationGroup *> (handle)->Free (_bytesCount, lock);
    }
}

AllocationGroup AllocationGroup::Parent () const noexcept
{
    return handle ? AllocationGroup {static_cast<Original::AllocationGroup *> (handle)->Parent ()} : AllocationGroup {};
}

Iterator AllocationGroup::BeginChildren () const noexcept
{
    Original::AllocationGroup::Iterator iterator =
        handle ? static_cast<Original::AllocationGroup *> (handle)->BeginChildren () :
                 Original::AllocationGroup::EndChildren ();
    return Iterator (array_cast (iterator));
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is part of the API.
Iterator AllocationGroup::EndChildren () const noexcept
{
    Original::AllocationGroup::Iterator iterator = Original::AllocationGroup::EndChildren ();
    return Iterator (array_cast (iterator));
}

UniqueString AllocationGroup::GetId () const noexcept
{
    if (handle)
    {
        return static_cast<Original::AllocationGroup *> (handle)->GetId ();
    }

    return {};
}

size_t AllocationGroup::GetAcquired () const noexcept
{
    if (handle)
    {
        return static_cast<Original::AllocationGroup *> (handle)->GetAcquired ();
    }

    return 0u;
}

size_t AllocationGroup::GetReserved () const noexcept
{
    if (handle)
    {
        return static_cast<Original::AllocationGroup *> (handle)->GetReserved ();
    }

    return 0u;
}

size_t AllocationGroup::GetTotal () const noexcept
{
    if (handle)
    {
        return static_cast<Original::AllocationGroup *> (handle)->GetTotal ();
    }

    return 0u;
}

uintptr_t AllocationGroup::Hash () const noexcept
{
    if (handle)
    {
        return static_cast<Original::AllocationGroup *> (handle)->Hash ();
    }

    return 0u;
}

AllocationGroup &AllocationGroup::operator= (const AllocationGroup &_other) noexcept = default;

AllocationGroup &AllocationGroup::operator= (AllocationGroup &&_other) noexcept = default;

bool AllocationGroup::operator== (const AllocationGroup &_other) const noexcept
{
    return handle == _other.handle;
}

bool AllocationGroup::operator!= (const AllocationGroup &_other) const noexcept
{
    return !(*this == _other);
}

AllocationGroup::AllocationGroup (void *_handle) noexcept
    : handle (_handle)
{
}
} // namespace Emergence::Memory::Profiler
