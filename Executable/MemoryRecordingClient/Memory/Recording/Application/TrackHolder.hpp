#pragma once

#include <fstream>

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <Memory/Recording/Constants.hpp>
#include <Memory/Recording/StreamDeserializer.hpp>
#include <Memory/Recording/Track.hpp>

namespace Emergence::Memory::Recording::Application
{
class TrackHolder final
{
public:
    using MarkerVector = Container::Vector<Track::EventIterator>;

    void Open (const char *_fileName) noexcept;

    void Close () noexcept;

    void Update () noexcept;

    [[nodiscard]] bool IsLoading () const noexcept;

    [[nodiscard]] const Track &GetTrack () const noexcept;

    [[nodiscard]] const MarkerVector &GetMarkers () const noexcept;

    [[nodiscard]] std::pair<MarkerVector::const_iterator, MarkerVector::const_iterator> GetMarkersInBounds (
        float _minS, float _maxS) const noexcept;

    [[nodiscard]] double GetMarkerFrequency (UniqueString _markerId) const noexcept;

    /// \see Track::MoveToPreviousEvent
    bool MoveToPreviousEvent () noexcept;

    /// \see Track::MoveToNextEvent
    bool MoveToNextEvent () noexcept;

private:
    struct MarkerFrequencyData
    {
        double accumulator = 0.0;
        double previousMarkerTimeS = 0.0;
        std::size_t count = 0u;
    };

    std::ifstream input;
    Track track;
    StreamDeserializer deserializer;

    // TODO: This vector will be very huge if recording is big. Think about other data structures.
    MarkerVector markers {Constants::AllocationGroup ()};

    Container::HashMap<UniqueString, MarkerFrequencyData> markerFrequency {Constants::AllocationGroup ()};

    bool fileOpen = false;
    bool loading = false;
};
} // namespace Emergence::Memory::Recording::Application
