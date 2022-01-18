#pragma once

#include <Container/Optional.hpp>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/DeserializerBase.hpp>
#include <Memory/Recording/Event.hpp>
#include <Memory/Recording/SerializationHelpers.hpp>

namespace Emergence::Memory::Recording
{
class RuntimeReporter final : public DeserializerBase
{
public:
    RuntimeReporter () noexcept = default;

    RuntimeReporter (const RuntimeReporter &_other) = delete;

    RuntimeReporter (RuntimeReporter &&_other) noexcept;

    ~RuntimeReporter () noexcept;

    void Begin (Recording *_target, const Profiler::CapturedAllocationGroup &_capturedRoot) noexcept;

    void ReportEvent (const Profiler::Event &_event) noexcept;

    void End () noexcept;

    RuntimeReporter &operator= (const RuntimeReporter &_other) = delete;

    RuntimeReporter &operator= (RuntimeReporter &&_other) noexcept;

private:
    GroupUIDAssigner uidAssigner;
};
} // namespace Emergence::Memory::Recording
