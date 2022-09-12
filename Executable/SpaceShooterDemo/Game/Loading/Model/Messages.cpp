#include <Loading/Model/Messages.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const LevelGenerationRequest::Reflection &LevelGenerationRequest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LevelGenerationRequest);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const LevelGenerationFinishedResponse::Reflection &LevelGenerationFinishedResponse::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LevelGenerationFinishedResponse);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
