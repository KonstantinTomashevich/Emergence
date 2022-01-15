#pragma once

#include <istream>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/DeserializerBase.hpp>
#include <Memory/Recording/Event.hpp>

namespace Emergence::Memory::Recording
{
class StreamDeserializer final : public DeserializerBase
{
public:
    StreamDeserializer () noexcept = default;

    StreamDeserializer (const StreamDeserializer &_other) = delete;

    StreamDeserializer (StreamDeserializer &&_other) noexcept;

    ~StreamDeserializer () noexcept;

    void Begin (Recording *_target, std::istream *_input) noexcept;

    bool TryReadNext () noexcept;

    void End () noexcept;

    StreamDeserializer &operator= (const StreamDeserializer &_other) = delete;

    StreamDeserializer &operator= (StreamDeserializer &&_other) noexcept;

private:
    bool ReadDeclareGroupEvent (std::uint64_t _timeNs) noexcept;

    bool ReadMemoryManagementEvent (EventType _eventType, std::uint64_t _timeNs) noexcept;

    bool ReadMarkerEvent (std::uint64_t _timeNs) noexcept;

    bool ReadUniqueString (UniqueString &_output) noexcept;

    std::istream *input = nullptr;
};
} // namespace Emergence::Memory::Recording
