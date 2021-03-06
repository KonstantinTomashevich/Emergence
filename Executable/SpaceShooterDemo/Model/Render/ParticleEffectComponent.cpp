#include <Render/ParticleEffectComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

ParticleEffectComponent::ParticleEffectComponent () noexcept = default;

ParticleEffectComponent::~ParticleEffectComponent () noexcept = default;

const ParticleEffectComponent::Reflection &ParticleEffectComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ParticleEffectComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (effectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (effectName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (effectTag);
        EMERGENCE_MAPPING_REGISTER_REGULAR (playing);
        EMERGENCE_MAPPING_REGISTER_REGULAR (implementationHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
