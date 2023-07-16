#pragma once

#include <atomic>

namespace Emergence::Handling
{
/// \brief Default implementation of Handleable concept.
///
/// \details Simplifies integration with Handle mechanism: allows Handle usage for any derived class.
class HandleableBase
{
public:
    /// It doesn't make sense to copy references counters.
    HandleableBase (const HandleableBase &_other) = delete;

    /// It doesn't make sense to move references counters.
    HandleableBase (HandleableBase &&_other) = delete;

    ~HandleableBase () = default;

    /// \brief Adds reference to counter.
    /// \invariant ::references should not overflow during increment.
    void RegisterReference () noexcept;

    /// \brief Subtracts reference from counter.
    /// \invariant ::references should be greater than zero.
    void UnregisterReference () noexcept;

    /// \return ::references
    [[nodiscard]] std::uintptr_t GetReferenceCount () const noexcept;

    /// It doesn't make sense to copy assign references counters.
    HandleableBase &operator= (const HandleableBase &_other) = delete;

    /// It doesn't make sense to move assign references counters.
    HandleableBase &operator= (HandleableBase &&_other) = delete;

protected:
    /// \brief Initializes ::references to zero.
    HandleableBase () = default;

private:
    /// \brief Internal reference counter.
    std::atomic_uintptr_t references {0u};
};
} // namespace Emergence::Handling
