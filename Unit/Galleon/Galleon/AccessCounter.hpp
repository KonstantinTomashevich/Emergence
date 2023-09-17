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
    AccessCounter () noexcept = default;

    AccessCounter (const AccessCounter &_other) = delete;

    AccessCounter (AccessCounter &&_other) = delete;

    ~AccessCounter () = default;

    /// \details Thread safe.
    void RegisterReadAccess () noexcept;

    /// \details Thread safe.
    void UnregisterReadAccess () noexcept;

    void RegisterWriteAccess () noexcept;

    void UnregisterWriteAccess () noexcept;

    void SetUnsafeFetchAllowed (bool _allowed) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (AccessCounter);

private:
    std::atomic_size_t read {0u};

    static_assert (decltype (read)::is_always_lock_free);

    std::size_t write {0u};

    bool unsafeFetchAllowed = false;
};
} // namespace Emergence::Galleon
