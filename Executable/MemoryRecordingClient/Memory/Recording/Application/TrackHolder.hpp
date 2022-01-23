#pragma once

#include <fstream>

#include <Container/Vector.hpp>

#include <Memory/Recording/Constants.hpp>
#include <Memory/Recording/StreamDeserializer.hpp>
#include <Memory/Recording/Track.hpp>

namespace Emergence::Memory::Recording::Application
{
class TrackHolder final
{
public:
    void Open (const char *_fileName) noexcept;

    void Close () noexcept;

    void Update () noexcept;

    [[nodiscard]] bool IsLoading () const noexcept;

    [[nodiscard]] const Track &GetTrack () const noexcept;

    [[nodiscard]] const Container::Vector<Track::EventIterator> &GetMarkers () const noexcept;

    /// \see Track::MoveToPreviousEvent
    bool MoveToPreviousEvent () noexcept;

    /// \see Track::MoveToNextEvent
    bool MoveToNextEvent () noexcept;

private:
    std::ifstream input;
    Track track;
    StreamDeserializer deserializer;
    Container::Vector<Track::EventIterator> markers {Constants::AllocationGroup ()};

    bool fileOpen = false;
    bool loading = false;
};
} // namespace Emergence::Memory::Recording::Application
