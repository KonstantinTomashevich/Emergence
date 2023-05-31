#pragma once

#include <array>

#include <StandardLayout/Field.hpp>
#include <StandardLayout/Mapping.hpp>

namespace Emergence::Warehouse::Test
{
// To simplify singleton-related testing logic we use struct with zero-filling default constructor.
struct TestSingleton
{
    static constexpr std::size_t DATA_ITEMS = 16u;

    struct Reflection final
    {
        std::array<StandardLayout::FieldId, DATA_ITEMS> data;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect ();

    static const TestSingleton NON_DEFAULT_INSTANCE;

    std::array<std::uint32_t, DATA_ITEMS> data = {0u};
};
} // namespace Emergence::Warehouse::Test
