#include <atomic>

#include <Assert/Assert.hpp>

#include <Memory/Profiler/ImplementationUtils.hpp>
#include <Memory/Profiler/Original/AllocationGroup.hpp>
#include <Memory/Profiler/Original/EventManager.hpp>
#include <Memory/UnorderedPool.hpp>

namespace Emergence::Memory::Profiler::Original
{
AllocationGroup *AllocationGroup::Iterator::operator* () const noexcept
{
    return current;
}

AllocationGroup::Iterator::Iterator (const AllocationGroup::Iterator &_other) noexcept = default;

AllocationGroup::Iterator::Iterator (AllocationGroup::Iterator &&_other) noexcept = default;

AllocationGroup::Iterator &AllocationGroup::Iterator::operator= (const AllocationGroup::Iterator &_other) noexcept =
    default;

AllocationGroup::Iterator &AllocationGroup::Iterator::operator= (AllocationGroup::Iterator &&_other) noexcept = default;

AllocationGroup::Iterator::~Iterator () noexcept = default;

AllocationGroup::Iterator &AllocationGroup::Iterator::operator++ () noexcept
{
    EMERGENCE_ASSERT (current);
    current = current->nextOnLevel;
    return *this;
}

AllocationGroup::Iterator AllocationGroup::Iterator::operator++ (int) noexcept
{
    Iterator previous = *this;
    ++*this;
    return previous;
}

bool AllocationGroup::Iterator::operator== (const AllocationGroup::Iterator &_other) const noexcept
{
    return current == _other.current;
}

bool AllocationGroup::Iterator::operator!= (const AllocationGroup::Iterator &_other) const noexcept
{
    return *this == _other;
}

AllocationGroup::Iterator::Iterator (AllocationGroup *_current) noexcept
    : current (_current)
{
}

static UnorderedPool &GetAllocationGroupPool () noexcept
{
    // There is no sense to profile memory usage of profiling classes, therefore we use stub-group.
    static UnorderedPool allocationGroupPool {Profiler::AllocationGroup {}, sizeof (AllocationGroup),
                                              alignof (AllocationGroup)};
    return allocationGroupPool;
}

AllocationGroup *AllocationGroup::Root () noexcept
{
    // Root allocation group does not need id, therefore its id is empty string.
    static auto *root = new (GetAllocationGroupPool ().Acquire ()) AllocationGroup {nullptr, UniqueString {}};
    return root;
}

AllocationGroup *AllocationGroup::Request (UniqueString _id, const ProfilingLock &_lock) noexcept
{
    return Request (AllocationGroupStack::Get ().Top (), _id, _lock);
}

AllocationGroup *AllocationGroup::Request (AllocationGroup *_parent,
                                           UniqueString _id,
                                           const ProfilingLock & /*unused*/) noexcept
{
    EMERGENCE_ASSERT (_parent);
    AllocationGroup *child = _parent->firstChild;

    while (child)
    {
        if (child->id == _id)
        {
            return child;
        }

        child = child->nextOnLevel;
    }

    auto *newGroup = new (GetAllocationGroupPool ().Acquire ()) AllocationGroup {_parent, _id};
    newGroup->nextOnLevel = _parent->firstChild;
    _parent->firstChild = newGroup;
    return newGroup;
}

void AllocationGroup::Allocate (size_t _bytesCount, const ProfilingLock &_lock) noexcept
{
    AllocateInternal (_bytesCount);
    EventManager::Get ().Allocate (ImplementationUtils::ToServiceFormat (this), _bytesCount, _lock);
}

void AllocationGroup::Acquire (size_t _bytesCount, const ProfilingLock &_lock) noexcept
{
    AcquireInternal (_bytesCount);
    EventManager::Get ().Acquire (ImplementationUtils::ToServiceFormat (this), _bytesCount, _lock);
}

void AllocationGroup::Release (size_t _bytesCount, const ProfilingLock &_lock) noexcept
{
    ReleaseInternal (_bytesCount);
    EventManager::Get ().Release (ImplementationUtils::ToServiceFormat (this), _bytesCount, _lock);
}

void AllocationGroup::Free (size_t _bytesCount, const ProfilingLock &_lock) noexcept
{
    FreeInternal (_bytesCount);
    EventManager::Get ().Free (ImplementationUtils::ToServiceFormat (this), _bytesCount, _lock);
}

AllocationGroup *AllocationGroup::Parent () const noexcept
{
    return parent;
}

AllocationGroup::Iterator AllocationGroup::BeginChildren () const noexcept
{
    return Iterator (firstChild);
}

AllocationGroup::Iterator AllocationGroup::EndChildren () noexcept
{
    return Iterator {nullptr};
}

UniqueString AllocationGroup::GetId () const noexcept
{
    return id;
}

size_t AllocationGroup::GetAcquired () const noexcept
{
    return acquired;
}

size_t AllocationGroup::GetReserved () const noexcept
{
    return reserved;
}

size_t AllocationGroup::GetTotal () const noexcept
{
    return acquired + reserved;
}

uintptr_t AllocationGroup::Hash () const noexcept
{
    // Groups are never deallocated, therefore address is a perfect hash.
    return reinterpret_cast<uintptr_t> (this);
}

void AllocationGroup::AllocateInternal (size_t _bytesCount) noexcept
{
    reserved += _bytesCount;
    if (parent)
    {
        parent->AllocateInternal (_bytesCount);
    }
}

void AllocationGroup::AcquireInternal (size_t _bytesCount) noexcept
{
    EMERGENCE_ASSERT (reserved >= _bytesCount);
    reserved -= _bytesCount;
    acquired += _bytesCount;

    if (parent)
    {
        parent->AcquireInternal (_bytesCount);
    }
}

void AllocationGroup::ReleaseInternal (size_t _bytesCount) noexcept
{
    EMERGENCE_ASSERT (acquired >= _bytesCount);
    acquired -= _bytesCount;
    reserved += _bytesCount;

    if (parent)
    {
        parent->ReleaseInternal (_bytesCount);
    }
}

void AllocationGroup::FreeInternal (size_t _bytesCount) noexcept
{
    EMERGENCE_ASSERT (reserved >= _bytesCount);
    reserved -= _bytesCount;

    if (parent)
    {
        parent->FreeInternal (_bytesCount);
    }
}

AllocationGroup::AllocationGroup (AllocationGroup *_parent, UniqueString _id) noexcept
    : id (_id),
      parent (_parent)
{
}

AllocationGroupStack &AllocationGroupStack::Get () noexcept
{
    static thread_local AllocationGroupStack instance;
    return instance;
}

void AllocationGroupStack::Push (AllocationGroup *_group) noexcept
{
    EMERGENCE_ASSERT (_group);
    stack.EmplaceBack (_group);
}

AllocationGroup *AllocationGroupStack::Top () noexcept
{
    return stack.Back ();
}

void AllocationGroupStack::Pop () noexcept
{
    stack.PopBack ();
}

AllocationGroupStack::AllocationGroupStack ()
{
    stack.EmplaceBack (AllocationGroup::Root ());
}
} // namespace Emergence::Memory::Profiler::Original
