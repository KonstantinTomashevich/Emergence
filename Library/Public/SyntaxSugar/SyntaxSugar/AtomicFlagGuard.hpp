#pragma once

#include <atomic>

namespace Emergence
{
/// \brief RAII guard for busy waiting with std::atomic_flag.
class AtomicFlagGuard final
{
public:
    AtomicFlagGuard (std::atomic_flag &_flag) noexcept;

    AtomicFlagGuard (const AtomicFlagGuard &_other) = delete;

    AtomicFlagGuard (AtomicFlagGuard &&_other) = delete;

    ~AtomicFlagGuard () noexcept;

    AtomicFlagGuard &operator= (const AtomicFlagGuard &_other) = delete;

    AtomicFlagGuard &operator= (AtomicFlagGuard &&_other) = delete;

private:
    std::atomic_flag &flag;
};

/// \brief Uses spinlock with yield to lock atomic flag.
void LockAtomicFlag (std::atomic_flag &_flag) noexcept;

/// \brief Unlocks atomic flag by clearing it.
void UnlockAtomicFlag (std::atomic_flag &_flag) noexcept;
} // namespace Emergence
