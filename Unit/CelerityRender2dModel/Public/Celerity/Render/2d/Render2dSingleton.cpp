#include <limits>

#include <Celerity/Render/2d/Render2dSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
uintptr_t Render2dSingleton::GenerateSprite2dId () const noexcept
{
    EMERGENCE_ASSERT (sprite2dIdCounter != std::numeric_limits<decltype (sprite2dIdCounter)::value_type>::max ());
    return const_cast<Render2dSingleton *> (this)->sprite2dIdCounter++;
}

uintptr_t Render2dSingleton::GenerateDebugShape2dId () const noexcept
{
    EMERGENCE_ASSERT (debugShape2dIdCounter !=
                      std::numeric_limits<decltype (debugShape2dIdCounter)::value_type>::max ());
    return const_cast<Render2dSingleton *> (this)->debugShape2dIdCounter++;
}

const Render2dSingleton::Reflection &Render2dSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Render2dSingleton);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
