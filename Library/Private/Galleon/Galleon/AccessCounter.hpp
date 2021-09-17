#pragma once

#include <atomic>
#include <cstdint>

#include <API/Common/Shortcuts.hpp>

namespace Emergence::Galleon
{
/// \brief Asserts readers-writers access rules.
class AccessCounter final
{
public:
    AccessCounter () noexcept;

    AccessCounter (const AccessCounter &_other) = delete;

    AccessCounter (AccessCounter &&_other) noexcept;

    /// \details Thread safe.
    void RegisterReadAccess () noexcept;

    /// \details Thread safe.
    void UnregisterReadAccess () noexcept;

    void RegisterWriteAccess () noexcept;

    void UnregisterWriteAccess () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (AccessCounter);

private:
    std::atomic_size_t read;

    static_assert (decltype (read)::is_always_lock_free);

    std::size_t write;
};
} // namespace Emergence::Galleon