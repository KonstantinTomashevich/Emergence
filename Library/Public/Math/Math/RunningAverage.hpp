#pragma once

#include <Container/InplaceVector.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Math
{
/// \brief Implements updatable running average.
/// \tparam SampleSize Maximum count of samples.
template <std::size_t SampleSize>
class RunningAverage final
{
public:
    /// \brief Add new sample to the average. If there is no space, removes oldest sample.
    void Push (float _sample) noexcept;

    /// \return Current running average.
    [[nodiscard]] float Get () const noexcept;

private:
    float average = 0.0f;
    Container::InplaceVector<float, SampleSize> samples;
    std::size_t nextSampleIndex = 0u;

public:
    struct Reflection final
    {
        // There is no need to expose internal calculation buffer to reflection. Therefore, we expose only result.
        StandardLayout::FieldId average;
        StandardLayout::Mapping mapping;
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
} // namespace Emergence::Math
