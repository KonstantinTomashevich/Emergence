#pragma once

#include <chrono>
#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

#include <Memory/Profiler/Event.hpp>
#include <Memory/Profiler/Group.hpp>

namespace Emergence::Memory::Profiler
{
class Core final
{
public:
    Core () = delete;

    static void PushGroupPrefix (const char *_prefix) noexcept;

    static void PopGroupPrefix () noexcept;

    static void SetJournalFileName (const char *_fileName) noexcept;

    static GroupRegistry::Iterator BeginGroups () noexcept;

    static GroupRegistry::Iterator EndGroups () noexcept;

    static EventIterator BeginCurrentEvents () noexcept;

    static EventIterator EndCurrentEvents () noexcept;
};

class GroupPrefixScope final
{
public:
    inline GroupPrefixScope (const char *_prefix) noexcept
    {
        Core::PushGroupPrefix (_prefix);
    }

    inline ~GroupPrefixScope () noexcept
    {
        Core::PopGroupPrefix ();
    }
};
} // namespace Emergence::Memory::Profiler
