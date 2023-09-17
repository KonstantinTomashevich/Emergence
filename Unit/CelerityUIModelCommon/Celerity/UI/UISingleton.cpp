#include <limits>

#include <Celerity/UI/UISingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
uintptr_t UISingleton::GenerateNodeId () const noexcept
{
    EMERGENCE_ASSERT (nodeIdCounter != std::numeric_limits<decltype (nodeIdCounter)::value_type>::max ());
    return const_cast<UISingleton *> (this)->nodeIdCounter++;
}

const UISingleton::Reflection &UISingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UISingleton);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
