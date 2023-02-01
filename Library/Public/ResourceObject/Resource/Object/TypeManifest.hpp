#pragma once

#include <Container/HashMap.hpp>

#include <Resource/Object/AllocationGroup.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Resource::Object
{
/// \brief This type should be used to uniquely identify object parts for inheritance algorithms.
using UniqueId = std::uint64_t;

/// \brief Contains metadata attached to register object part type.
struct TypeInfo final
{
    /// \brief Field that contains unique id value for object part of this type.
    /// \details Id must be unique among all parts of the same type, but parts of different types may have equal ids.
    StandardLayout::FieldId uniqueId;
};

/// \brief Contains metadata for all types, instances of which can be used as object parts.
class TypeManifest final
{
public:
    /// \brief Enables injection feature and initializes data for it.
    /// \details Injection provides ability to implement composition in resource objects by specifying injected
    ///          sub objects through specified injector type that holds id of sub object in injector id field.
    ///          Injected object data is not directly added to owner changelist, instead injected object type
    ///          is loaded into object library like any other object, so user instantiation library can implement
    ///          composition by manually instantiating injected object from found injector instances.
    void InitInjection (const StandardLayout::Mapping &_injectorType,
                        StandardLayout::FieldId _injectorIdField) noexcept;

    /// \brief Registers metadata for given type.
    void Register (const StandardLayout::Mapping &_mapping, const TypeInfo &_info) noexcept;

    /// \return Metadata for given type or `nullptr` if it's not registered.
    [[nodiscard]] const TypeInfo *Get (const StandardLayout::Mapping &_mapping) const noexcept;

    /// \return Map of all registered types with their metadata.
    [[nodiscard]] const Container::HashMap<StandardLayout::Mapping, TypeInfo> &GetMap () const noexcept;

    /// \brief Injector type for injection feature. See ::InitInjection.
    [[nodiscard]] const StandardLayout::Mapping &GetInjectorType () const noexcept;

    /// \brief Injector sub object id field for injection feature. See ::InitInjection.
    [[nodiscard]] StandardLayout::FieldId GetInjectorIdField () const noexcept;

private:
    StandardLayout::Mapping injectorType;
    StandardLayout::FieldId injectorIdField = 0u;

    Container::HashMap<StandardLayout::Mapping, TypeInfo> infos {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"TypeManifest"}}};
};
} // namespace Emergence::Resource::Object
