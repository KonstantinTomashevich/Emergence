#include <cassert>

#include <Celerity/Model/WorldSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
uintptr_t WorldSingleton::GenerateUID () const noexcept
{
    assert (uidCounter != std::numeric_limits<decltype (uidCounter)::value_type>::max ());
    return const_cast<WorldSingleton *> (this)->uidCounter++;
}

const WorldSingleton::Reflection &WorldSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (WorldSingleton)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedUpdateHappened)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Celerity
