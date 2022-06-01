#include <Physics/PhysXAccessSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Physics
{
PhysXAccessSingleton::Reflection &PhysXAccessSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        Emergence::StandardLayout::MappingBuilder builder;
        builder.Begin (Emergence::Memory::UniqueString {"PhysXAccessSingleton"}, sizeof (PhysXAccessSingleton),
                       alignof (PhysXAccessSingleton));

        return PhysXAccessSingleton::Reflection {
            .mapping = builder.End (),
        };
    }();

    return reflection;
}
} // namespace Emergence::Physics
