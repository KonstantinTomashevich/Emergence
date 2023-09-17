#pragma once

#include <ResourceObjectApi.hpp>

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <Resource/Object/AllocationGroup.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Resource::Object
{
/// \brief This type should be used to uniquely identify object parts for inheritance algorithms.
using UniqueId = std::uint64_t;

/// \brief Contains metadata attached to register object part type.
struct ResourceObjectApi TypeInfo final
{
    /// \brief Field that contains unique id value for object part of this type.
    /// \details Id must be unique among all parts of the same type, but parts of different types may have equal ids.
    StandardLayout::FieldId uniqueId;
};

/// \brief Describes injection feature for particular object part type.
/// \details Injection provides ability to implement composition in resource objects by specifying injected
///          sub objects through one of the injector types that hold id of sub object in injector id field.
///          Injected object data is not directly added to owner changelist, instead injected object type
///          is loaded into object library like any other object, so user instantiation library can implement
///          composition by manually instantiating injected object from found injector instances.
struct ResourceObjectApi DependencyInjectionInfo final
{
    /// \brief Type of an object part, that contains field with injection.
    StandardLayout::Mapping injectorType;

    /// \brief Field that contains injected object id.
    StandardLayout::FieldId injectorIdField = 0u;
};

/// \brief Contains metadata for all types, instances of which can be used as object parts.
class ResourceObjectApi TypeManifest final
{
public:
    /// \brief Enables injection feature for given type and field.
    void AddInjection (DependencyInjectionInfo _info) noexcept;

    /// \brief Registers metadata for given type.
    void Register (const StandardLayout::Mapping &_mapping, const TypeInfo &_info) noexcept;

    /// \return Metadata for given type or `nullptr` if it's not registered.
    [[nodiscard]] const TypeInfo *Get (const StandardLayout::Mapping &_mapping) const noexcept;

    /// \return Map of all registered types with their metadata.
    [[nodiscard]] const Container::HashMap<StandardLayout::Mapping, TypeInfo> &GetMap () const noexcept;

    /// \return Vector of all added injections.
    [[nodiscard]] const Container::Vector<DependencyInjectionInfo> &GetInjections () const noexcept;

private:
    Container::HashMap<StandardLayout::Mapping, TypeInfo> infos {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"TypeManifest"}}};

    Container::Vector<DependencyInjectionInfo> injections {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"TypeManifest"}}};
};
} // namespace Emergence::Resource::Object
