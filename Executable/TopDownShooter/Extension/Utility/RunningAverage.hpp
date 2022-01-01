#pragma once

#include <Container/InplaceVector.hpp>

#include <StandardLayout/Mapping.hpp>

#include <SyntaxSugar/MappingRegistration.hpp>

namespace Emergence
{
template <std::size_t SampleSize>
class RunningAverage final
{
public:
    void Push (float _sample) noexcept;

    [[nodiscard]] float Get () const noexcept;

private:
    float average = 0.0f;
    Container::InplaceVector<float, SampleSize> samples;
    std::size_t nextSampleIndex = 0u;

public:
    struct Reflection final
    {
        // There is no need to expose internal calculation buffer to reflection. Therefore, we expose only result.
        Emergence::StandardLayout::FieldId average;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

template <std::size_t SampleSize>
void RunningAverage<SampleSize>::Push (float _sample) noexcept
{
    if (samples.GetCount () < samples.GetCapacity ())
    {
        average = (average * static_cast<float> (samples.GetCount ()) + _sample) /
                  static_cast<float> (samples.GetCount () + 1u);
        samples.EmplaceBack (_sample);
    }
    else
    {
        average += (_sample - samples[nextSampleIndex]) / static_cast<float> (SampleSize);
        samples[nextSampleIndex] = _sample;
        nextSampleIndex = (nextSampleIndex + 1u) % SampleSize;
    }
}

template <std::size_t SampleSize>
float RunningAverage<SampleSize>::Get () const noexcept
{
    return average;
}

template <std::size_t SampleSize>
const typename RunningAverage<SampleSize>::Reflection &RunningAverage<SampleSize>::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (RunningAverage)
        EMERGENCE_MAPPING_REGISTER_REGULAR (average)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence
