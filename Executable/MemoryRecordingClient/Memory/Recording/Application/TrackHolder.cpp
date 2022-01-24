#include <chrono>

#include <Memory/Recording/Application/TrackHolder.hpp>

namespace Emergence::Memory::Recording::Application
{
void TrackHolder::Open (const char *_fileName) noexcept
{
    Close ();

    input.open (_fileName, std::ios::binary);
    deserializer.Begin (&track, &input);

    loading = true;
    fileOpen = true;
}

void TrackHolder::Close () noexcept
{
    if (!fileOpen)
    {
        return;
    }

    track.Clear ();
    deserializer.End ();
    input.close ();

    markers.clear ();
    markerFrequency.clear ();

    loading = false;
    fileOpen = true;
}

void TrackHolder::Update () noexcept
{
    using namespace std::chrono_literals;
    using Clock = std::chrono::high_resolution_clock;
    static const Clock ::duration timeout = 20ms;

    if (!fileOpen)
    {
        return;
    }

    input.sync ();
    const Clock::time_point updateStart = Clock::now ();
    loading = true;

    while (true)
    {
        if (!deserializer.TryReadNext ())
        {
            loading = false;
            break;
        }

        Track::EventIterator parsed = --track.EventEnd ();
        if ((*parsed)->type == EventType::MARKER)
        {
            markers.emplace_back (parsed);
            MarkerFrequencyData &frequency = markerFrequency[(*parsed)->markerId];
            double markerTimeS = static_cast<double> ((*parsed)->timeNs) * 1e-9;

            if (frequency.count > 1u)
            {
                frequency.accumulator = (frequency.accumulator * static_cast<double> (frequency.count - 1u) +
                                         markerTimeS - frequency.previousMarkerTimeS) /
                                        static_cast<double> (frequency.count);
            }

            frequency.previousMarkerTimeS = markerTimeS;
            ++frequency.count;
        }

        if (Clock::now () - updateStart > timeout)
        {
            break;
        }
    }
}

bool TrackHolder::IsLoading () const noexcept
{
    return loading;
}

const Track &TrackHolder::GetTrack () const noexcept
{
    return track;
}

const Container::Vector<Track::EventIterator> &TrackHolder::GetMarkers () const noexcept
{
    return markers;
}

std::pair<TrackHolder::MarkerVector::const_iterator, TrackHolder::MarkerVector::const_iterator>
TrackHolder::GetMarkersInBounds (float _minS, float _maxS) const noexcept
{
    auto lower = std::lower_bound (markers.begin (), markers.end (), _minS,
                                   [] (const Track::EventIterator &_event, float _border)
                                   {
                                       return static_cast<float> ((*_event)->timeNs) * 1e-9f < _border;
                                   });

    auto upper = std::upper_bound (markers.begin (), markers.end (), _maxS,
                                   [] (float _border, const Track::EventIterator &_event)
                                   {
                                       return _border < static_cast<float> ((*_event)->timeNs) * 1e-9f;
                                   });

    return {lower, upper};
}

double TrackHolder::GetMarkerFrequency (UniqueString _markerId) const noexcept
{
    auto iterator = markerFrequency.find (_markerId);
    if (iterator != markerFrequency.end () && iterator->second.count > 1u)
    {
        return iterator->second.accumulator;
    }

    return std::numeric_limits<double>::max ();
}

bool TrackHolder::MoveToPreviousEvent () noexcept
{
    return track.MoveToPreviousEvent ();
}

bool TrackHolder::MoveToNextEvent () noexcept
{
    return track.MoveToNextEvent ();
}
} // namespace Emergence::Memory::Recording::Application
