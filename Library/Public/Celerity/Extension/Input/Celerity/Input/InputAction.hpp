#pragma once

#include <array>
#include <cstdint>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Describes how action should be delivered to the user objects.
enum class InputActionDispatchType : uint8_t
{
    /// \brief Action should be dispatched in normal update and thrown away after being dispatched once.
    NORMAL = 0u,

    /// \brief Action should be dispatched in fixed update and thrown away after being dispatched once.
    /// \details Useful for single click actions like "Fire" or "ExecuteAbility".
    FIXED_INSTANT,

    /// \brief Action should be dispatched in every fixed update until next normal update.
    /// \brief Useful for dispatching continuous actions like movement when several fixed
    ///        updates may happen between two normal updates. Otherwise, in case of two or
    ///        more fixed updates input would be lost and character would stop moving.
    FIXED_PERSISTENT,
};

/// \brief Describes input action with gameplay-specific id and optional discrete or real parameters.
struct InputAction final
{
    /// \brief Maximum count of discrete parameters.
    static constexpr const size_t MAX_DISCRETE_PARAMETERS = 3u;

    /// \brief Maximum count of real parameters.
    static constexpr const size_t MAX_REAL_PARAMETERS = 3u;

    InputAction () noexcept;

    InputAction (const Memory::UniqueString &_group, const Memory::UniqueString &_id) noexcept;

    InputAction (const Memory::UniqueString &_group,
                 const Memory::UniqueString &_id,
                 const std::array<int32_t, MAX_DISCRETE_PARAMETERS> &_discrete) noexcept;

    InputAction (const Memory::UniqueString &_group,
                 const Memory::UniqueString &_id,
                 const std::array<float, MAX_REAL_PARAMETERS> &_real) noexcept;

    InputAction (const InputAction &_other) noexcept;

    InputAction (InputAction &&_other) noexcept;

    ~InputAction () = default;

    /// \brief Groups are used categorize actions so one object can subscribe to all actions from required group.
    Memory::UniqueString group;

    /// \brief Action id is unique identifier that has gameplay-related meaning and is independent of physical input.
    /// \details For example, "SwitchWeapon", "Fire", "Crouch" and so on.
    Memory::UniqueString id;

    union
    {
        /// \brief Action discrete parameters if any.
        std::array<int32_t, MAX_DISCRETE_PARAMETERS> discrete;

        /// \brief Action real parameters if any.
        std::array<float, MAX_REAL_PARAMETERS> real;

        static_assert (sizeof (discrete) == sizeof (real));
    };

    bool operator== (const InputAction &_other) const;

    bool operator!= (const InputAction &_other) const;

    InputAction &operator= (const InputAction &_other) noexcept;

    InputAction &operator= (InputAction &&_other) noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId group;
        StandardLayout::FieldId id;
        std::array<StandardLayout::FieldId, MAX_DISCRETE_PARAMETERS> discrete;
        std::array<StandardLayout::FieldId, MAX_REAL_PARAMETERS> real;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
