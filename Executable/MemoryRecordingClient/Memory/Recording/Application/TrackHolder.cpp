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

    timeSelectionRequestS.reset ();
    selectedGroup = nullptr;

    loading = false;
    fileOpen = true;
}

void TrackHolder::Update () noexcept
{
    UpdateLoading ();
    UpdateTimeSelection ();
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

float TrackHolder::GetSelectedTimeS () const noexcept
{
    return GetEventTime (track.EventCurrent ());
    ;
}

void TrackHolder::SelectTime (float _seconds) noexcept
{
    timeSelectionRequestS = _seconds;
}

const RecordedAllocationGroup *TrackHolder::GetSelectedGroup () const noexcept
{
    return selectedGroup;
}

void TrackHolder::SelectGroup (const RecordedAllocationGroup *_group) noexcept
{
    selectedGroup = _group;
}

void TrackHolder::MoveBy (int _offset) noexcept
{
    if (_offset < 0)
    {
        while (_offset)
        {
            ++_offset;
            if (!track.MoveToPreviousEvent ())
            {
                return;
            }
        }
    }
    else
    {
        while (_offset)
        {
            --_offset;
            if (!track.MoveToNextEvent ())
            {
                return;
            }
        }
    }
}

void TrackHolder::UpdateLoading () noexcept
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

void TrackHolder::UpdateTimeSelection () noexcept
{
    using namespace std::chrono_literals;
    using Clock = std::chrono::high_resolution_clock;
    static const Clock ::duration timeout = 10ms;

    if (timeSelectionRequestS)
    {
        const Clock::time_point updateStart = Clock::now ();
        Track::EventIterator currentEvent = track.EventCurrent ();
        const float currentTimeS = GetEventTime (currentEvent);
        const bool goingForward = timeSelectionRequestS.value () > currentTimeS;

        while (true)
        {
            if (goingForward)
            {
                ++currentEvent;
                if (GetEventTime (currentEvent) > timeSelectionRequestS.value () || !track.MoveToNextEvent ())
                {
                    timeSelectionRequestS.reset ();
                    break;
                }
            }
            else
            {
                --currentEvent;
                const bool lastUndo = GetEventTime (currentEvent) < timeSelectionRequestS.value ();

                if (!track.MoveToPreviousEvent () || lastUndo)
                {
                    timeSelectionRequestS.reset ();
                    break;
                }
            }

            if (Clock::now () - updateStart > timeout)
            {
                break;
            }
        }
    }
}

float TrackHolder::GetEventTime (const Track::EventIterator &_iterator) const noexcept
{
    return _iterator == track.EventEnd () ? 0.0f : static_cast<float> ((*_iterator)->timeNs) * 1e-9f;
}
} // namespace Emergence::Memory::Recording::Application
