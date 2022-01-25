#pragma once

#include <fstream>

#include <Container/HashMap.hpp>
#include <Container/Optional.hpp>
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

    [[nodiscard]] float GetSelectedTimeS () const noexcept;

    void SelectTime (float _seconds) noexcept;

    [[nodiscard]] const RecordedAllocationGroup *GetSelectedGroup () const noexcept;

    void SelectGroup (const RecordedAllocationGroup *_group) noexcept;

    /// \brief Immediately move by **small** offset.
    void MoveBy (int _offset) noexcept;

private:
    struct MarkerFrequencyData
    {
        double accumulator = 0.0;
        double previousMarkerTimeS = 0.0;
        std::size_t count = 0u;
    };

    void UpdateLoading () noexcept;

    void UpdateTimeSelection () noexcept;

    [[nodiscard]] float GetEventTime (const Track::EventIterator &_iterator) const noexcept;

    std::ifstream input;
    Track track;
    StreamDeserializer deserializer;

    // TODO: This vector will be very huge if recording is big. Think about other data structures.
    MarkerVector markers {Constants::AllocationGroup ()};

    Container::HashMap<UniqueString, MarkerFrequencyData> markerFrequency {Constants::AllocationGroup ()};

    Container::Optional<float> timeSelectionRequestS;
    const RecordedAllocationGroup *selectedGroup = nullptr;

    bool fileOpen = false;
    bool loading = false;
};
} // namespace Emergence::Memory::Recording::Application
