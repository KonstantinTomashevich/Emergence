#include <Assert/Assert.hpp>

#include <Gameplay/PlayerInfoSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

uintptr_t PlayerInfoSingleton::GeneratePlayerId () const noexcept
{
    EMERGENCE_ASSERT (playerUidCounter != std::numeric_limits<decltype (playerUidCounter)::value_type>::max ());
    return const_cast<PlayerInfoSingleton *> (this)->playerUidCounter++;
}

const PlayerInfoSingleton::Reflection &PlayerInfoSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PlayerInfoSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (localPlayerUid);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
