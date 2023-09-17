#pragma once

#include <CelerityPhysics3dModelPhysXApi.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Internal singleton, used to register fetch and modify access to PhysX engine objects.
struct CelerityPhysics3dModelPhysXApi PhysXAccessSingleton final
{
    struct CelerityPhysics3dModelPhysXApi Reflection
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
