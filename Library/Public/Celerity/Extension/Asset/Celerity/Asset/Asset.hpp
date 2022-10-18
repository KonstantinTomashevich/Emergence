#pragma once

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
enum class AssetState : uint8_t
{
    LOADING,
    MISSING,
    CORRUPTED,
    READY,
};

struct Asset final
{
    Memory::UniqueString id;
    StandardLayout::Mapping type;
    std::uint32_t usages = 0u;
    AssetState state = AssetState::LOADING;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId usages;
        StandardLayout::FieldId state;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
