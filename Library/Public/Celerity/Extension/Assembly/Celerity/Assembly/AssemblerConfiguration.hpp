#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
constexpr UniqueId ASSEMBLY_ROOT_OBJECT_ID = 0u;

constexpr UniqueId ASSEMBLY_OBJECT_ID_KEY_INDEX = std::numeric_limits<UniqueId>::max ();

Memory::Profiler::AllocationGroup GetAssemblyConfigurationAllocationGroup () noexcept;

struct CustomKeyDescriptor final
{
    using ProviderMethod = UniqueId (*) (const void *);

    StandardLayout::Mapping singletonProviderType;
    ProviderMethod providerMethod;
};

struct KeyBinding final
{
    StandardLayout::Field keyField;
    UniqueId keyIndex;
};

struct TypeDescriptor final
{
    StandardLayout::Mapping type;
    Container::Vector<KeyBinding> keys {GetAssemblyConfigurationAllocationGroup ()};
    Container::Vector<StandardLayout::Field> localVector3fs {GetAssemblyConfigurationAllocationGroup ()};
};

struct AssemblerConfiguration final
{
    Container::Vector<CustomKeyDescriptor> customKeys {GetAssemblyConfigurationAllocationGroup ()};
    Container::Vector<TypeDescriptor> types {GetAssemblyConfigurationAllocationGroup ()};
};
} // namespace Emergence::Celerity
