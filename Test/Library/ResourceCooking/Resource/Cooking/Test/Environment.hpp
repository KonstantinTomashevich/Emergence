#pragma once

#include <Resource/Cooking/Context.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

namespace Emergence::Resource::Cooking::Test
{
struct FirstObjectType final
{
    Memory::UniqueString configId;
    std::uint32_t width = 0u;
    std::uint32_t height = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId configId;
        StandardLayout::FieldId width;
        StandardLayout::FieldId height;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct SecondObjectType final
{
    Memory::UniqueString configId;
    float cost = 0.0f;
    float time = 0.0f;

    struct Reflection final
    {
        StandardLayout::FieldId configId;
        StandardLayout::FieldId cost;
        StandardLayout::FieldId time;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

template <typename Data>
struct ResourceInfo
{
    using DataType = Data;

    Container::Utf8String path;
    Data data;
};

struct Environment final
{
    Container::Vector<ResourceInfo<FirstObjectType>> objectsFirst;
    Container::Vector<ResourceInfo<SecondObjectType>> objectsSecond;
    Container::Vector<ResourceInfo<Container::Vector<std::uint8_t>>> thirdParty;
};

const Container::MappingRegistry &GetResourceObjectTypes () noexcept;

void PrepareEnvironmentAndSetupContext (Context &_context, const Environment &_environment);

Container::Utf8String GetFinalResultRealPath (const Context &_context, const std::string_view &_resultName) noexcept;
} // namespace Emergence::Resource::Cooking::Test
