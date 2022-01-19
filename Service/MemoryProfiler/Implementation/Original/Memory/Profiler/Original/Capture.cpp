#include <cassert>

#include <Memory/Profiler/Original/Capture.hpp>

namespace Emergence::Memory::Profiler::Original
{
Handling::Handle<CapturedAllocationGroup> CapturedAllocationGroup::Iterator::operator* () const noexcept
{
    return current;
}

CapturedAllocationGroup::Iterator::Iterator (const CapturedAllocationGroup::Iterator &_other) noexcept = default;

CapturedAllocationGroup::Iterator::Iterator (CapturedAllocationGroup::Iterator &&_other) noexcept = default;

CapturedAllocationGroup::Iterator &CapturedAllocationGroup::Iterator::operator= (
    const CapturedAllocationGroup::Iterator &_other) noexcept = default;

CapturedAllocationGroup::Iterator &CapturedAllocationGroup::Iterator::operator= (
    CapturedAllocationGroup::Iterator &&_other) noexcept = default;

CapturedAllocationGroup::Iterator::~Iterator () noexcept = default;

CapturedAllocationGroup::Iterator &CapturedAllocationGroup::Iterator::operator++ () noexcept
{
    assert (current);
    current = current->nextOnLevel;
    return *this;
}

CapturedAllocationGroup::Iterator CapturedAllocationGroup::Iterator::operator++ (int) noexcept
{
    Iterator previous = *this;
    ++*this;
    return previous;
}

bool CapturedAllocationGroup::Iterator::operator== (const CapturedAllocationGroup::Iterator &_other) const noexcept
{
    return current == _other.current;
}

bool CapturedAllocationGroup::Iterator::operator!= (const CapturedAllocationGroup::Iterator &_other) const noexcept
{
    return !(*this == _other);
}

CapturedAllocationGroup::Iterator::Iterator (Handling::Handle<CapturedAllocationGroup> _current) noexcept
    : current (std::move (_current))
{
}

static UnorderedPool &GetCapturedAllocationGroupPool ()
{
    static UnorderedPool pool {Profiler::AllocationGroup {}, sizeof (CapturedAllocationGroup)};
    return pool;
}

void *CapturedAllocationGroup::operator new (std::size_t /*unused*/) noexcept
{
    return GetCapturedAllocationGroupPool ().Acquire ();
}

void CapturedAllocationGroup::operator delete (void *_pointer) noexcept
{
    GetCapturedAllocationGroupPool ().Release (_pointer);
}

CapturedAllocationGroup::CapturedAllocationGroup (const AllocationGroup &_source,
                                                  const ProfilingLock &_lock,
                                                  uint64_t _sharedCaptureTime) noexcept
    : id (_source.GetId ()),
      reserved (_source.GetReserved ()),
      acquired (_source.GetAcquired ()),
      firstChild (_source.firstChild ? new CapturedAllocationGroup {*_source.firstChild, _lock, _sharedCaptureTime} :
                                       nullptr),
      nextOnLevel (_source.nextOnLevel ? new CapturedAllocationGroup {*_source.nextOnLevel, _lock, _sharedCaptureTime} :
                                         nullptr),
      source (&_source),
      captureTimeNs (_sharedCaptureTime)
{
}

CapturedAllocationGroup::Iterator CapturedAllocationGroup::BeginChildren () const noexcept
{
    return Iterator {firstChild};
}

CapturedAllocationGroup::Iterator CapturedAllocationGroup::EndChildren () noexcept
{
    return Iterator {nullptr};
}

UniqueString CapturedAllocationGroup::GetId () const noexcept
{
    return id;
}

size_t CapturedAllocationGroup::GetAcquired () const noexcept
{
    return acquired;
}

size_t CapturedAllocationGroup::GetReserved () const noexcept
{
    return reserved;
}

size_t CapturedAllocationGroup::GetTotal () const noexcept
{
    return acquired + reserved;
}

const AllocationGroup *CapturedAllocationGroup::GetSource () const noexcept
{
    return source;
}

uint64_t CapturedAllocationGroup::GetCaptureTimeNs () const noexcept
{
    return captureTimeNs;
}

EventObserver::EventObserver (const ProfilingLock &_lock) noexcept
    : current (EventManager::Get ().StartObservation (_lock))
{
}

EventObserver::EventObserver (EventObserver &&_other) noexcept : current (_other.current)
{
    _other.current = nullptr;
    _other.movedOut = true;
}

EventObserver::~EventObserver () noexcept
{
    if (!movedOut)
    {
        ProfilingLock lock;
        EventManager::Get ().FinishObservation (current, lock);
    }
}

const Event *EventObserver::NextEvent (const ProfilingLock &_lock) noexcept
{
    assert (!movedOut);
    if (const EventNode *next = EventManager::Get ().RequestNext (current, _lock))
    {
        current = next;
        return &next->event;
    }

    return nullptr;
}
} // namespace Emergence::Memory::Profiler::Original
