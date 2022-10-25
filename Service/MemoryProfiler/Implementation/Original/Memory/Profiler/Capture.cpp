#include <API/Common/Implementation/Iterator.hpp>

#include <Assert/Assert.hpp>

#include <Memory/Profiler/Capture.hpp>
#include <Memory/Profiler/ImplementationUtils.hpp>
#include <Memory/Profiler/Original/Capture.hpp>
#include <Memory/Profiler/Original/ProfilingLock.hpp>

#include <SyntaxSugar/BlockCast.hpp>
#include <SyntaxSugar/Time.hpp>

namespace Emergence::Memory::Profiler
{
void AddMarker (UniqueString _markerId, const AllocationGroup &_group) noexcept
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
    EMERGENCE_ASSERT (group);
    auto iterator = group->BeginChildren ();
    return Iterator (array_cast (iterator));
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is part of the API.
Iterator CapturedAllocationGroup::EndChildren () const noexcept
{
    auto iterator = Original::CapturedAllocationGroup::EndChildren ();
    return Iterator (array_cast (iterator));
}

UniqueString CapturedAllocationGroup::GetId () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    EMERGENCE_ASSERT (group);
    return group->GetId ();
}

size_t CapturedAllocationGroup::GetAcquired () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    EMERGENCE_ASSERT (group);
    return group->GetAcquired ();
}

size_t CapturedAllocationGroup::GetReserved () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    EMERGENCE_ASSERT (group);
    return group->GetReserved ();
}

size_t CapturedAllocationGroup::GetTotal () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    EMERGENCE_ASSERT (group);
    return group->GetTotal ();
}

AllocationGroup CapturedAllocationGroup::GetSource () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    EMERGENCE_ASSERT (group);
    return ImplementationUtils::ToServiceFormat (const_cast<Original::AllocationGroup *> (group->GetSource ()));
}

uint64_t CapturedAllocationGroup::GetCaptureTimeNs () const noexcept
{
    const CapturedGroupHandle &group = *reinterpret_cast<const CapturedGroupHandle *> (&handle);
    EMERGENCE_ASSERT (group);
    return group->GetCaptureTimeNs ();
}

CapturedAllocationGroup::CapturedAllocationGroup (void *_handle) noexcept
{
    EMERGENCE_ASSERT (_handle);
    static_assert (sizeof (handle) == sizeof (CapturedGroupHandle));
    new (&handle) CapturedGroupHandle (static_cast<Original::CapturedAllocationGroup *> (_handle));
}

EventObserver::EventObserver (EventObserver &&_other) noexcept
{
    new (&data) Original::EventObserver (std::move (block_cast<Original::EventObserver> (_other.data)));
}

EventObserver::~EventObserver () noexcept
{
    block_cast<Original::EventObserver> (data).~EventObserver ();
}

const Event *EventObserver::NextEvent () noexcept
{
    Original::ProfilingLock lock;
    return block_cast<Original::EventObserver> (data).NextEvent (lock);
}

EventObserver::EventObserver (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Original::EventObserver (std::move (block_cast<Original::EventObserver> (_data)));
}

std::pair<CapturedAllocationGroup, EventObserver> Capture::Start () noexcept
{
    Original::ProfilingLock lock;
    // Capture is done in one transaction, therefore reported capture time is equal for all groups.
    const uint64_t sharedCaptureTime = Time::NanosecondsSinceStartup ();

    auto *capturedRoot =
        new Original::CapturedAllocationGroup {*Original::AllocationGroup::Root (), lock, sharedCaptureTime};
    Original::EventObserver observer {lock};

    return {CapturedAllocationGroup {capturedRoot}, EventObserver {array_cast (observer)}};
}
} // namespace Emergence::Memory::Profiler
