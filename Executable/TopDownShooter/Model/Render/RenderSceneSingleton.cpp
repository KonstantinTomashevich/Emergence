#include <Render/RenderSceneSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

RenderSceneSingleton::RenderSceneSingleton () noexcept = default;

RenderSceneSingleton::~RenderSceneSingleton () noexcept = default;

const RenderSceneSingleton::Reflection &RenderSceneSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (RenderSceneSingleton)
        EMERGENCE_MAPPING_REGISTER_REGULAR (cameraObjectId)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
