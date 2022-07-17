#pragma once

#include <array>
#include <cstdint>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

struct InputAction final
{
    static constexpr const std::size_t MAX_DISCRETE_PARAMETERS = 3u;
    static constexpr const std::size_t MAX_REAL_PARAMETERS = 3u;

    InputAction () noexcept = default;

    InputAction (const Emergence::Memory::UniqueString &_group, const Emergence::Memory::UniqueString &_id) noexcept;

    InputAction (const Emergence::Memory::UniqueString &_group,
                 const Emergence::Memory::UniqueString &_id,
                 const std::array<int32_t, MAX_DISCRETE_PARAMETERS> &_discrete) noexcept;

    InputAction (const Emergence::Memory::UniqueString &_group,
                 const Emergence::Memory::UniqueString &_id,
                 const std::array<float, MAX_REAL_PARAMETERS> &_real) noexcept;

    Emergence::Memory::UniqueString group;
    Emergence::Memory::UniqueString id;

    union
    {
        std::array<int32_t, MAX_DISCRETE_PARAMETERS> discrete;
        std::array<float, MAX_REAL_PARAMETERS> real;
    };

    bool operator== (const InputAction &_other) const;

    bool operator!= (const InputAction &_other) const;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId group;
        Emergence::StandardLayout::FieldId id;
        std::array<Emergence::StandardLayout::FieldId, MAX_DISCRETE_PARAMETERS> discrete;
        std::array<Emergence::StandardLayout::FieldId, MAX_REAL_PARAMETERS> real;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
