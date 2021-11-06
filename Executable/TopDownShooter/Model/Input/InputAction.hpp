#pragma once

#include <array>
#include <cstdint>

#include <StandardLayout/Mapping.hpp>

struct InputAction final
{
    constexpr static const std::size_t MAX_DISCRETE_PARAMETERS = 3u;
    constexpr static const std::size_t MAX_REAL_PARAMETERS = 3u;

    InputAction () noexcept = default;

    InputAction (uint64_t _id) noexcept;

    InputAction (uint64_t _id, const std::array<int32_t, MAX_DISCRETE_PARAMETERS> &_discrete) noexcept;

    InputAction (uint64_t _id, const std::array<float, MAX_REAL_PARAMETERS> &_real) noexcept;

    uint64_t id = 0u;

    union
    {
        std::array<int32_t, MAX_DISCRETE_PARAMETERS> discrete;
        std::array<float, MAX_REAL_PARAMETERS> real;
    };

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId id;
        std::array<Emergence::StandardLayout::FieldId, MAX_DISCRETE_PARAMETERS> discrete;
        std::array<Emergence::StandardLayout::FieldId, MAX_REAL_PARAMETERS> real;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
