#include <cassert>

#include <API/Common/Implementation/Iterator.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Memory/Profiler/Capture.hpp>
#include <Memory/Profiler/Original/Capture.hpp>
#include <Memory/Profiler/Original/ProfilingLock.hpp>

namespace Emergence::Memory::Profiler
{
void AddMarker (UniqueString _markerId, AllocationGroup _group) noexcept
{
    Original::ProfilingLock lock;
    Original::EventManager::Get ().Marker (_group, _markerId, lock);
}

using Iterator = CapturedAllocationGroup::Iterator;

using IteratorImplementation = Original::CapturedAllocationGroup::Iterator;

CapturedAllocationGroup CapturedAllocationGroup::Iterator::operator* () const noexcept
{
    return CapturedAllocationGroup ((*block_cast<IteratorImplementation> (data)).Get ());
}

EMERGENCE_BIND_FORWARD_ITERATOR_OPERATIONS_IMPLEMENTATION (Iterator, IteratorImplementation)

using CapturedGroupHandle = Handling::Handle<Original::CapturedAllocationGroup>;

CapturedAllocationGroup::CapturedAllocationGroup (const CapturedAllocationGroup &_other) noexcept
{
    new (&handle) CapturedGroupHandle (*reinterpret_cast<const CapturedGroupHandle *> (&_other.handle));
}

CapturedAllocationGroup::CapturedAllocationGroup (CapturedAllocationGroup &&_other) noexcept
{
    new (&handle) CapturedGroupHandle (std::move (*reinterpret_cast<CapturedGroupHandle *> (&_other.handle)));
}

CapturedAllocationGroup::~CapturedAllocationGroup () noexcept
{
    reinterpret_cast<CapturedGroupHandle *> (&handle)->~CapturedGroupHandle ();
}

Iterator CapturedAllocationGroup::BeginChildren () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    assert (group);
    auto iterator = group->BeginChildren ();
    return Iterator (reinterpret_cast<decltype (Iterator::data) *> (&iterator));
}

Iterator CapturedAllocationGroup::EndChildren () const noexcept
{
    auto iterator = Original::CapturedAllocationGroup::EndChildren ();
    return Iterator (reinterpret_cast<decltype (Iterator::data) *> (&iterator));
}

UniqueString CapturedAllocationGroup::GetId () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    assert (group);
    return group->GetId ();
}

size_t CapturedAllocationGroup::GetAcquired () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    assert (group);
    return group->GetAcquired ();
}

size_t CapturedAllocationGroup::GetReserved () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    assert (group);
    return group->GetReserved ();
}

size_t CapturedAllocationGroup::GetTotal () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    assert (group);
    return group->GetTotal ();
}

CapturedAllocationGroup::CapturedAllocationGroup (void *_handle) noexcept
{
    assert (_handle);
    static_assert (sizeof (handle) == sizeof (CapturedGroupHandle));
    new (&handle) CapturedGroupHandle (static_cast<Original::CapturedAllocationGroup *> (_handle));
}

EventObserver::EventObserver (EventObserver &&_other) noexcept
{
    new (&data) Original::EventObserver (std::move (block_cast<Original::EventObserver> (_other.data)));
}

const Event *EventObserver::NextEvent () noexcept
{
    Original::ProfilingLock lock;
    return block_cast<Original::EventObserver> (data).NextEvent (lock);
}

EventObserver::EventObserver (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Original::EventObserver (std::move (block_cast<Original::EventObserver> (*_data)));
}

std::pair<CapturedAllocationGroup, EventObserver> Capture::Start () noexcept
{
    Original::ProfilingLock lock;
    auto *capturedRoot = new Original::CapturedAllocationGroup {*Original::AllocationGroup::Root (), lock};
    Original::EventObserver observer {lock};

    return {CapturedAllocationGroup {capturedRoot},
            EventObserver {reinterpret_cast<decltype (EventObserver::data) *> (&observer)}};
}
} // namespace Emergence::Memory::Profiler
