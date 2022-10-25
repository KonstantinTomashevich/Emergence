#include <Assert/Assert.hpp>

#include <Celerity/Model/WorldSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
uintptr_t WorldSingleton::GenerateId () const noexcept
{
    EMERGENCE_ASSERT (idCounter != std::numeric_limits<decltype (idCounter)::value_type>::max ());
    return const_cast<WorldSingleton *> (this)->idCounter++;
}

const WorldSingleton::Reflection &WorldSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (WorldSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedUpdateHappened);
        EMERGENCE_MAPPING_REGISTER_REGULAR (updateMode);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
