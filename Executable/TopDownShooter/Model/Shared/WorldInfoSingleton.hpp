#pragma once

#include <cstdint>

#include <StandardLayout/Mapping.hpp>

struct WorldInfoSingleton final
{
    /// \brief Indicates whether current normal update was separated from previous one by one or more fixed updates.
    /// \warning Access outside of normal update routine leads to undefined behaviour.
    bool fixedUpdateHappened = false;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId fixedUpdateHappened;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
