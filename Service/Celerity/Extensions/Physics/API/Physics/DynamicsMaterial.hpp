#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Physics
{
/// \warning Dynamics material removal causes removal of all associated shapes,
///          because shapes can not exist without material.
struct DynamicsMaterial final
{
    EMERGENCE_STATIONARY_DATA_TYPE (DynamicsMaterial);

    Memory::UniqueString id;

    float dynamicFriction = 0.0f;

    float staticFriction = 0.0f;

    bool enableFriction = true;

    float restitution = 0.0f;

    float density = 0.0f;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId dynamicFriction;
        StandardLayout::FieldId staticFriction;
        StandardLayout::FieldId enableFriction;
        StandardLayout::FieldId restitution;
        StandardLayout::FieldId density;
        StandardLayout::FieldId implementationHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Physics
