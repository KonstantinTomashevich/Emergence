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

bool TrackHolder::MoveToPreviousEvent () noexcept
{
    return track.MoveToPreviousEvent ();
}

bool TrackHolder::MoveToNextEvent () noexcept
{
    return track.MoveToNextEvent ();
}
} // namespace Emergence::Memory::Recording::Application
