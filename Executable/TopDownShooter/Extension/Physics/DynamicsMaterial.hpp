#pragma once

#include <cstdint>

#include <API/Common/Shortcuts.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Physics
{
struct DynamicsMaterial final
{
    DynamicsMaterial () = default;

    DynamicsMaterial (const DynamicsMaterial &_other) = delete;

    DynamicsMaterial (DynamicsMaterial &&_other) = delete;

    ~DynamicsMaterial() noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (DynamicsMaterial);

    uint64_t id = 0u;

    float dynamicFriction = 0.0f;

    float staticFriction = 0.0f;

    bool enableFriction = true;

    float restitution = 0.0f;

    float density = 0.0f;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId id;
        Emergence::StandardLayout::FieldId dynamicFriction;
        Emergence::StandardLayout::FieldId staticFriction;
        Emergence::StandardLayout::FieldId enableFriction;
        Emergence::StandardLayout::FieldId restitution;
        Emergence::StandardLayout::FieldId density;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
