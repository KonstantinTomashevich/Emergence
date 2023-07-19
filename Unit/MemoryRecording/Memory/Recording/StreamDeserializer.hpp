#pragma once

#include <MemoryRecordingApi.hpp>

#include <istream>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/Event.hpp>
#include <Memory/Recording/ReporterBase.hpp>

namespace Emergence::Memory::Recording
{
/// \brief Deserializes events from standard stream and reports them into Track.
class MemoryRecordingApi StreamDeserializer final : public ReporterBase
{
public:
    StreamDeserializer () noexcept = default;

    StreamDeserializer (const StreamDeserializer &_other) = delete;

    StreamDeserializer (StreamDeserializer &&_other) noexcept;

    ~StreamDeserializer () noexcept;

    /// \brief Begin deserializing events from given stream and reporting them into given track.
    /// \invariant Input stream should not be destructed before deserialization sessions ends.
    /// \invariant Not reporting into another track already.
    void Begin (Track *_target, std::istream *_input) noexcept;

    /// \brief Tries to deserialize next event from the stream and report it.
    /// \return True if event was deserialized successfully.
    /// \invariant Currently deserializing and reporting events to some track.
    bool TryReadNext () noexcept;

    /// \brief Ends deserialization session.
    /// \invariant Currently deserializing and reporting events to some track.
    void End () noexcept;

    StreamDeserializer &operator= (const StreamDeserializer &_other) = delete;

    StreamDeserializer &operator= (StreamDeserializer &&_other) noexcept;

private:
    std::istream *input = nullptr;
};
} // namespace Emergence::Memory::Recording
