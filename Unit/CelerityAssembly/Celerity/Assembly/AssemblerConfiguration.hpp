#pragma once

#include <limits>

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Components that are added to root object (the one that has PrototypeComponent)
///        should have this value in object id key field.
constexpr UniqueId ASSEMBLY_ROOT_OBJECT_ID = 0u;

/// \brief Indicates to KeyBinding::keyIndex that binding reference predefined object id key instead of any custom key.
constexpr UniqueId ASSEMBLY_OBJECT_ID_KEY_INDEX = std::numeric_limits<UniqueId>::max ();

/// \return Allocation group used to allocate all assembly configuration structures.
Memory::Profiler::AllocationGroup GetAssemblerConfigurationAllocationGroup () noexcept;

/// \brief Provides ability to register custom keys, for example instance ids for multi components
///        (components is called multi component when multiple instances can be attached to one object).
/// \details Only keys with prototype-internal values (like object ids) should be registered like that.
///          If reference is global then assembler does not recalculate its id and therefore has nothing
///          to do with this key. Examples of global references: models, unit configs.
struct CustomKeyDescriptor final
{
    /// \brief Signature of a function used to get new id from id generator singleton.
    using ProviderFunction = UniqueId (*) (const void *);

    /// \brief Type of a singleton used to generate new values for this custom key.
    StandardLayout::Mapping singletonProviderType;

    /// \brief Function applied to instance of ::singletonProviderType to get new id.
    ProviderFunction providerFunction;
};

/// \brief Informs that field contains value of selected key.
struct KeyBinding final
{
    /// \brief Id of a field, that holds value associated with selected key.
    StandardLayout::FieldId keyField;

    /// \brief Index of custom key or ::ASSEMBLY_OBJECT_ID_KEY_INDEX.
    UniqueId keyIndex;
};

/// \brief Describes component type that can be spawned during assembly routine.
struct TypeDescriptor final
{
    /// \brief Mapping of this component type.
    StandardLayout::Mapping type;

    /// \brief Lists all fields that are associated with keys.
    Container::Vector<KeyBinding> keys {GetAssemblerConfigurationAllocationGroup ()};

    /// \brief Lists all fields of type Vector3f that should be rotated
    ///        by root object world rotation after component spawn.
    /// \details In some rare cases it is useful to apply this transformation. For example, it allows to specify
    ///          bullet velocity as rigid body velocity inside patch without any additional components.
    Container::Vector<StandardLayout::FieldId> rotateVector3fs {GetAssemblerConfigurationAllocationGroup ()};
};

/// \brief Lists all user-defined keys.
using CustomKeyVector = Container::Vector<CustomKeyDescriptor>;

/// \brief Lists all component types that can be spawned during assembly routine.
using TypeBindingVector = Container::Vector<TypeDescriptor>;
} // namespace Emergence::Celerity
