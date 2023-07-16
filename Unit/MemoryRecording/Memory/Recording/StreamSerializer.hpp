#pragma once

#include <ostream>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/Event.hpp>
#include <Memory/Recording/ReportingHelpers.hpp>

namespace Emergence::Memory::Recording
{
/// \brief Serializes MemoryProfiler events using MemoryRecording format into standard stream.
class StreamSerializer final
{
public:
    StreamSerializer () noexcept = default;

    StreamSerializer (const StreamSerializer &_other) = delete;

    StreamSerializer (StreamSerializer &&_other) noexcept;

    ~StreamSerializer () noexcept;

    /// \brief Begins serialization session into given stream and writes given captured group hierarchy.
    /// \invariant Output stream should not be destructed before serialization sessions ends.
    /// \invariant Not serializing to another stream already.
    void Begin (std::ostream *_output, const Profiler::CapturedAllocationGroup &_capturedRoot) noexcept;

    /// \brief Serializes given event into target output stream.
    /// \invariant Currently serializing events into some stream.
    void SerializeEvent (const Profiler::Event &_event) noexcept;

    /// \brief Ends serialization session.
    /// \invariant Currently serializing events into some stream.
    void End () noexcept;

    StreamSerializer &operator= (const StreamSerializer &_other) = delete;

    StreamSerializer &operator= (StreamSerializer &&_other) noexcept;

private:
    std::ostream *output = nullptr;
    GroupUIDAssigner uidAssigner;
};
} // namespace Emergence::Memory::Recording
