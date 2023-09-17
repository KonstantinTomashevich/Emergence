#include <GameCore/GameStateSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const GameStateRequest::Reflection &GameStateRequest::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (GameStateRequest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (state);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (state, 3u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (levelName);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const GameStateSingleton::Reflection &GameStateSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (GameStateSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (state);
        EMERGENCE_MAPPING_REGISTER_REGULAR (request);
        EMERGENCE_MAPPING_REGISTER_REGULAR (lastRequest);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
