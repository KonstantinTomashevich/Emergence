#include <Render/RenderSceneSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

uintptr_t RenderSceneSingleton::GenerateLightUID () const noexcept
{
    assert (lightUIDCounter != std::numeric_limits<decltype (lightUIDCounter)::value_type>::max ());
    return const_cast<RenderSceneSingleton *> (this)->lightUIDCounter++;
}

uintptr_t RenderSceneSingleton::GenerateModelUID () const noexcept
{
    assert (modelUIDCounter != std::numeric_limits<decltype (modelUIDCounter)::value_type>::max ());
    return const_cast<RenderSceneSingleton *> (this)->modelUIDCounter++;
}

uintptr_t RenderSceneSingleton::GenerateEffectUID () const noexcept
{
    assert (effectUIDCounter != std::numeric_limits<decltype (effectUIDCounter)::value_type>::max ());
    return const_cast<RenderSceneSingleton *> (this)->effectUIDCounter++;
}

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
