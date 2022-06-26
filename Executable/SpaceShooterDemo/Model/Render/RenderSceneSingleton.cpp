#include <Render/RenderSceneSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

uintptr_t RenderSceneSingleton::GenerateLightId () const noexcept
{
    assert (lightIdCounter != std::numeric_limits<decltype (lightIdCounter)::value_type>::max ());
    return const_cast<RenderSceneSingleton *> (this)->lightIdCounter++;
}

uintptr_t RenderSceneSingleton::GenerateModelId () const noexcept
{
    assert (modelIdCounter != std::numeric_limits<decltype (modelIdCounter)::value_type>::max ());
    return const_cast<RenderSceneSingleton *> (this)->modelIdCounter++;
}

uintptr_t RenderSceneSingleton::GenerateEffectId () const noexcept
{
    assert (effectIdCounter != std::numeric_limits<decltype (effectIdCounter)::value_type>::max ());
    return const_cast<RenderSceneSingleton *> (this)->effectIdCounter++;
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
