#include <Gameplay/RandomSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

float RandomSingleton::Next () noexcept
{
    return std::uniform_real_distribution<float> {0.0f, 1.0f}(generator);
}

float RandomSingleton::NextInRange (float _min, float _max) noexcept
{
    return _min + Next () * (_max - _min);
}

const RandomSingleton::Reflection &RandomSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (RandomSingleton)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
