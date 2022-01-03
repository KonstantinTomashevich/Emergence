#include <Memory/Profiler/Capture.hpp>
#include <Memory/Profiler/Registry.hpp>

namespace Emergence::Memory::Profiler
{
// TODO: Empty stubs are temporary inserted for linking.

GroupPrefix::GroupPrefix ([[maybe_unused]] UniqueString _id) noexcept
{
}

GroupPrefix::~GroupPrefix () noexcept
{
}

bool Registry::Iterator::operator== ([[maybe_unused]] const Registry::Iterator &_other) const noexcept
{
    return false;
}

Registry::Iterator &Registry::Iterator::operator++ () noexcept
{
    return *this;
}

Registry::Iterator Registry::Iterator::operator++ (int) noexcept
{
    return Registry::Iterator (nullptr);
}

Registry Registry::Iterator::operator* () const noexcept
{
    return Registry (Memory::UniqueString ());
}

Registry::Iterator::Iterator ([[maybe_unused]] const Registry::Iterator &_other) noexcept
{
}

Registry::Iterator::Iterator ([[maybe_unused]] Registry::Iterator &&_other) noexcept
{
}

Registry::Iterator &Registry::Iterator::operator= ([[maybe_unused]] const Registry::Iterator &_other) noexcept
{
    return *this;
}
Registry::Iterator &Registry::Iterator::operator= ([[maybe_unused]] Registry::Iterator &&_other) noexcept
{
    return *this;
}

Registry::Iterator::~Iterator () noexcept
{
}

bool Registry::Iterator::operator!= ([[maybe_unused]] const Registry::Iterator &_other) const noexcept
{
    return false;
}

Registry::Iterator::Iterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    data = *_data;
}

Registry::Registry ([[maybe_unused]] UniqueString _id) noexcept
{
}

Registry::Iterator Registry::BeginGlobal () noexcept
{
    return Registry::Iterator (nullptr);
}

Registry::Iterator Registry::EndGlobal () noexcept
{
    return Registry::Iterator (nullptr);
}

Registry::Registry ([[maybe_unused]] const Registry &_other) noexcept
{
}

Registry::Registry ([[maybe_unused]] Registry &&_other) noexcept
{
}

Registry::~Registry () noexcept
{
}

void Registry::Allocate ([[maybe_unused]] size_t _bytesCount) noexcept
{
}

void Registry::Acquire ([[maybe_unused]] size_t _bytesCount) noexcept
{
}

void Registry::Release ([[maybe_unused]] size_t _bytesCount) noexcept
{
}

void Registry::Free ([[maybe_unused]] size_t _bytesCount) noexcept
{
}

Registry::Iterator Registry::BeginChildren () noexcept
{
    return Registry::Iterator (nullptr);
}

Registry::Iterator Registry::EndChildren () noexcept
{
    return Registry::Iterator (nullptr);
}

UniqueString Registry::GetId () const noexcept
{
    return UniqueString ();
}

size_t Registry::GetAcquired () const noexcept
{
    return 0;
}

size_t Registry::GetReserved () const noexcept
{
    return 0;
}

size_t Registry::GetTotal () const noexcept
{
    return 0;
}

Registry &Registry::operator= ([[maybe_unused]] const Registry &_other) noexcept
{
    return *this;
}

Registry &Registry::operator= ([[maybe_unused]] Registry &&_other) noexcept
{
    return *this;
}

Registry::Registry (void *_handle) noexcept
{
    handle = _handle;
}
} // namespace Emergence::Memory::Profiler
