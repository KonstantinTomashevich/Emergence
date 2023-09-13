#pragma once

#include <CelerityApi.hpp>

#include <atomic>
#include <cstdint>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Singleton for world<->tasks communication and global utility like id generation.
/// \warning This singleton is modified by World outside of pipeline execution,
///          therefore OnChange events do not work with it.
struct CelerityApi WorldSingleton final
{
    // TODO: Right now field below is only used for death spiral detection algorithm. But it looks sketchy.
    //       Maybe, rework death spiral detection algorithm and get rid of it?
    //       It is actually connected to time management that is planned to be updated in the future.

    /// \brief Indicates whether current normal update was separated from previous one by one or more fixed updates.
    /// \warning Access outside of normal update routine leads to undefined behaviour.
    bool fixedUpdateHappened = false;

    /// \brief Count of existing context escapes that are currently being used outside of Celerity.
    std::atomic_uintptr_t contextEscapeCounter = 0u;

    /// \brief Whether it is allowed to escape Celerity context right now.
    /// \details Some framework features might work only if there are no context escapes. In that cases they
    ///          need to forbid context escape using this flag and wait till existing escapes return.
    bool contextEscapeAllowed = true;

    /// \invariant Do not access directly, use ::GenerateId.
    std::atomic_uintptr_t idCounter = 0u;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    std::uintptr_t GenerateId () const noexcept;

    struct CelerityApi Reflection final
    {
        StandardLayout::FieldId fixedUpdateHappened;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
