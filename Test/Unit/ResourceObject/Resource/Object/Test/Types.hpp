#pragma once

#include <Container/MappingRegistry.hpp>

#include <Resource/Object/TypeManifest.hpp>

namespace Emergence::Resource::Object::Test
{
struct FirstComponent final
{
    std::uint64_t objectId = 0u;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId z;
        StandardLayout::FieldId w;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct SecondComponent final
{
    std::uint64_t objectId = 0u;
    std::uint64_t health = 0u;
    std::uint64_t money = 0u;
    std::uint64_t experience = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId health;
        StandardLayout::FieldId money;
        StandardLayout::FieldId experience;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct MultiComponent final
{
    std::uint64_t objectId = 0u;
    std::uint64_t instanceId = 0u;
    Memory::UniqueString modelId;
    Memory::UniqueString materialId;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId instanceId;
        StandardLayout::FieldId modelId;
        StandardLayout::FieldId materialId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct InjectionComponent final
{
    std::uint64_t objectId = 0u;
    Memory::UniqueString injectionId;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId injectionId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const TypeManifest &GetTypeManifest () noexcept;

const Container::MappingRegistry &GetResourceObjectMappingRegistry () noexcept;

const Container::MappingRegistry &GetPatchableTypesMappingRegistry () noexcept;
} // namespace Emergence::Resource::Object::Test
