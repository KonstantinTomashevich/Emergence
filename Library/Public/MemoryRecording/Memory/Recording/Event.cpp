#include <cassert>

#include <Memory/Recording/Event.hpp>

namespace Emergence::Memory::Recording
{
Event::Event (std::uint64_t _timeNs,
              GroupUID _parent,
              UniqueString _id,
              GroupUID _uid,
              std::uint64_t _reservedBytes,
              std::uint64_t _acquiredBytes) noexcept
    : type (EventType::DECLARE_GROUP),
      timeNs (_timeNs),
      parent (_parent),
      id (_id),
      uid (_uid),
      reservedBytes (_reservedBytes),
      acquiredBytes (_acquiredBytes)
{
}

Event::Event (EventType _type, std::uint64_t _timeNs, GroupUID _group, std::uint64_t _bytes) noexcept
    : type (_type),
      timeNs (_timeNs),
      group (_group),
      bytes (_bytes)
{
    assert (type == EventType::ALLOCATE || type == EventType::ACQUIRE || type == EventType::RELEASE ||
            type == EventType::FREE);
}

Event::Event (std::uint64_t _timeNs, GroupUID _scope, UniqueString _markedId) noexcept
    : type (EventType::MARKER),
      timeNs (_timeNs),
      scope (_scope),
      markerId (_markedId)
{
}
} // namespace Emergence::Memory::Recording
