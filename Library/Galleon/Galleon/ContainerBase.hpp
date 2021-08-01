#pragma once

#include <atomic>
#include <cstdint>

#include <API/Common/Shortcuts.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Galleon
{
class ContainerBase : public Handling::HandleableBase
{
public:
    /// Containers are managed by CargoDeck, therefore they can not be copied or moved.
    ContainerBase (const ContainerBase &_other) = delete;

    ContainerBase (ContainerBase &&_other) = delete;

    const StandardLayout::Mapping &GetTypeMapping () const noexcept;

    /// Containers are managed by CargoDeck, therefore they can not be assigned.
    EMERGENCE_DELETE_ASSIGNMENT (ContainerBase);

protected:
    explicit ContainerBase (class CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept;

    ~ContainerBase () noexcept = default;

    /// \details Thread safe.
    void RegisterReadCursor () noexcept;

    /// \details Thread safe.
    void UnregisterReadCursor () noexcept;

    void RegisterModificationCursor () noexcept;

    void UnregisterModificationCursor () noexcept;

    class CargoDeck *deck;

    StandardLayout::Mapping typeMapping;

    std::atomic_size_t readCursorCount;

    static_assert (decltype (readCursorCount)::is_always_lock_free);

    std::size_t modificationCursorCount;
};
} // namespace Emergence::Galleon