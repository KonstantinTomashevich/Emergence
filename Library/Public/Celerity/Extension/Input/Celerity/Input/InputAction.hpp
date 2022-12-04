#pragma once

#include <array>
#include <cstdint>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
enum class InputActionDispatchType : uint8_t
{
    NORMAL = 0u,
    FIXED_INSTANT,
    FIXED_PERSISTENT,
};

struct InputAction final
{
    static constexpr const size_t MAX_DISCRETE_PARAMETERS = 3u;
    static constexpr const size_t MAX_REAL_PARAMETERS = 3u;

    InputAction () noexcept;

    InputAction (const Memory::UniqueString &_group, const Memory::UniqueString &_id) noexcept;

    InputAction (const Memory::UniqueString &_group,
                 const Memory::UniqueString &_id,
                 const std::array<int32_t, MAX_DISCRETE_PARAMETERS> &_discrete) noexcept;

    InputAction (const Memory::UniqueString &_group,
                 const Memory::UniqueString &_id,
                 const std::array<float, MAX_REAL_PARAMETERS> &_real) noexcept;

    Memory::UniqueString group;
    Memory::UniqueString id;

    union
    {
        std::array<int32_t, MAX_DISCRETE_PARAMETERS> discrete;
        std::array<float, MAX_REAL_PARAMETERS> real;
    };

    bool operator== (const InputAction &_other) const;

    bool operator!= (const InputAction &_other) const;

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
