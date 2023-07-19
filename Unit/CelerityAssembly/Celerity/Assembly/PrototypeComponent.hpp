#pragma once

#include <CelerityAssemblyApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Enables assembly routine for this object by connecting it with assembly descriptor.
struct CelerityAssemblyApi PrototypeComponent final
{
    /// \brief Id of the object to which this component is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Id of AssemblyDescriptor using which object will be assembled.
    Memory::UniqueString descriptorId;

    /// \brief Whether fixed pipeline components must be assembled all at once during next frame.
    /// \details By default, fixed pipeline components contain logic and therefore must be instanced immediately.
    ///          If you can afford to delay fixed components loading, for example you're loading map chunk,
    ///          it is advised to make this false in order to split loading process to several frames.
    bool requestImmediateFixedAssembly = true;

    /// \brief Whether normal pipeline components must be assembled all at once during next frame.
    /// \details By default, normal pipeline components contain visuals and therefore their instancing can be delayed.
    bool requestImmediateNormalAssembly = false;

    /// \brief Whether assembly process was started for this component. For internal use only.
    bool assemblyStarted = false;

    struct CelerityAssemblyApi Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId descriptorId;
        StandardLayout::FieldId requestImmediateFixedAssembly;
        StandardLayout::FieldId requestImmediateNormalAssembly;
        StandardLayout::FieldId assemblyStarted;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
