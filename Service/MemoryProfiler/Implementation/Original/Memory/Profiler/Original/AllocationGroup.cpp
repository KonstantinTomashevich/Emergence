#include <atomic>
#include <cassert>
#include <thread>

#include <Memory/Profiler/Original/AllocationGroup.hpp>
#include <Memory/UnorderedPool.hpp>

#include <SyntaxSugar/AtomicFlagGuard.hpp>

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
    assert (current);
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

AllocationGroup::Iterator::Iterator (AllocationGroup *_current) noexcept : current (_current)
{
}

static UnorderedPool &GetAllocationGroupPool () noexcept
{
    // There is no sense to profile memory usage of profiling classes, therefore we use stub-group.
    static UnorderedPool allocationGroupPool {Profiler::AllocationGroup {}, sizeof (AllocationGroup)};
    return allocationGroupPool;
}

AllocationGroup *AllocationGroup::Root () noexcept
{
    // Root allocation group does not need id, therefore its id is empty string.
    static auto *root = new (GetAllocationGroupPool ().Acquire ()) AllocationGroup {nullptr, UniqueString {}};
    return root;
}

AllocationGroup *AllocationGroup::Request (UniqueString _id) noexcept
{
    return Request (AllocationGroupStack::Get ().Top (), _id);
}

AllocationGroup *AllocationGroup::Request (AllocationGroup *_parent, UniqueString _id) noexcept
{
    static std::atomic_flag groupAcquisition;
    AtomicFlagGuard guard {groupAcquisition};

    assert (_parent);
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

static std::atomic_flag &RecordingMemoryEvent ()
{
    static std::atomic_flag flag;
    return flag;
}

void AllocationGroup::Allocate (size_t _bytesCount) noexcept
{
    AtomicFlagGuard guard {RecordingMemoryEvent ()};
    AllocateInternal (_bytesCount);

    // TODO: Send event.
}

void AllocationGroup::Acquire (size_t _bytesCount) noexcept
{
    AtomicFlagGuard guard {RecordingMemoryEvent ()};
    AcquireInternal (_bytesCount);

    // TODO: Send event.
}

void AllocationGroup::Release (size_t _bytesCount) noexcept
{
    AtomicFlagGuard guard {RecordingMemoryEvent ()};
    ReleaseInternal (_bytesCount);

    // TODO: Send event.
}

void AllocationGroup::Free (size_t _bytesCount) noexcept
{
    AtomicFlagGuard guard {RecordingMemoryEvent ()};
    FreeInternal (_bytesCount);

    // TODO: Send event.
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
    assert (reserved >= _bytesCount);
    reserved -= _bytesCount;
    acquired += _bytesCount;

    if (parent)
    {
        parent->AcquireInternal (_bytesCount);
    }
}

void AllocationGroup::ReleaseInternal (size_t _bytesCount) noexcept
{
    assert (acquired >= _bytesCount);
    acquired -= _bytesCount;
    reserved += _bytesCount;

    if (parent)
    {
        parent->ReleaseInternal (_bytesCount);
    }
}

void AllocationGroup::FreeInternal (size_t _bytesCount) noexcept
{
    assert (reserved >= _bytesCount);
    reserved -= _bytesCount;

    if (parent)
    {
        parent->FreeInternal (_bytesCount);
    }
}

AllocationGroup::AllocationGroup (AllocationGroup *_parent, UniqueString _id) noexcept : id (_id), parent (_parent)
{
}

AllocationGroupStack &AllocationGroupStack::Get () noexcept
{
    static thread_local AllocationGroupStack instance;
    return instance;
}

void AllocationGroupStack::Push (AllocationGroup *_group) noexcept
{
    assert (_group);
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
