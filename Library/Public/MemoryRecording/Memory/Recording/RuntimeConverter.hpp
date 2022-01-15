#pragma once

#include <Container/Optional.hpp>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/DeserializerBase.hpp>
#include <Memory/Recording/Event.hpp>
#include <Memory/Recording/SerializationHelpers.hpp>

namespace Emergence::Memory::Recording
{
class RuntimeConverter final : public DeserializerBase
{
public:
    RuntimeConverter () noexcept = default;

    RuntimeConverter (const RuntimeConverter &_other) = delete;

    RuntimeConverter (RuntimeConverter &&_other) noexcept;

    ~RuntimeConverter () noexcept;

    void Begin (Recording *_target) noexcept;

    bool TryReadNext () noexcept;

    void End () noexcept;

    RuntimeConverter &operator= (const RuntimeConverter &_other) = delete;

    RuntimeConverter &operator= (RuntimeConverter &&_other) noexcept;

private:
    Container::Optional<Profiler::EventObserver> observer;
    GroupUIDAssigner uidAssigner;
};
} // namespace Emergence::Memory::Recording
