#pragma once

#include <Celerity/Assembly/AssemblerConfiguration.hpp>

#include <Math/Vector3f.hpp>

namespace Emergence::Celerity::Test
{
struct FixedComponentA final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    uint32_t x = 0u;
    uint32_t y = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct FixedComponentB final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    bool conditionA = false;
    bool conditionB = false;
    bool conditionC = false;
    bool conditionD = false;
    uint32_t q = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId conditionA;
        StandardLayout::FieldId conditionB;
        StandardLayout::FieldId conditionC;
        StandardLayout::FieldId conditionD;
        StandardLayout::FieldId q;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct FixedMultiComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId instanceId = INVALID_UNIQUE_ID;

    float m = 0.0f;
    float n = 0.0f;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId instanceId;
        StandardLayout::FieldId m;
        StandardLayout::FieldId n;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct FixedMultiComponentIdGeneratorSingleton final
{
    std::atomic_unsigned_lock_free instanceIdCounter = 0u;

    uintptr_t GenerateInstanceId () const noexcept;

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct VelocityFixedComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    Math::Vector3f globalVelocity;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId globalVelocity;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct NormalVisualComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    UniqueId mainMultiComponentId = INVALID_UNIQUE_ID;

    Memory::UniqueString model;
    Memory::UniqueString material;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId mainMultiComponentId;
        StandardLayout::FieldId model;
        StandardLayout::FieldId material;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

CustomKeyVector GetAssemblerCustomKeys () noexcept;

TypeBindingVector GetFixedAssemblerTypes () noexcept;

TypeBindingVector GetNormalAssemblerTypes () noexcept;
} // namespace Emergence::Celerity::Test
