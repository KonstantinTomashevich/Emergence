#pragma once

#include <ostream>

#include <Container/HashMap.hpp>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/Event.hpp>
#include <Memory/Recording/SerializationHelpers.hpp>

namespace Emergence::Memory::Recording
{
class StreamSerializer final
{
public:
    StreamSerializer () noexcept = default;

    StreamSerializer (const StreamSerializer &_other) = delete;

    StreamSerializer (StreamSerializer &&_other) noexcept;

    ~StreamSerializer () noexcept;

    void Begin (std::ostream *_output, const Profiler::CapturedAllocationGroup &_capturedRoot) noexcept;

    void SerializeEvent (const Profiler::Event &_event) noexcept;

    void End () noexcept;

    StreamSerializer &operator= (const StreamSerializer &_other) = delete;

    StreamSerializer &operator= (StreamSerializer &&_other) noexcept;

private:
    void WriteEvent (const Event &_event) noexcept;

    // TODO: Does almost the same thing as RuntimeConverter, but has different API.
    //       Think about merging APIs, maybe remove implicit capture from RuntimeConverter?

    std::ostream *output = nullptr;
    GroupUIDAssigner uidAssigner;
};
} // namespace Emergence::Memory::Recording
