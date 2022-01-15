#pragma once

#include <functional>

#include <Container/HashMap.hpp>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/Constants.hpp>
#include <Memory/Recording/Event.hpp>

namespace Emergence::Memory::Recording
{
class GroupUIDAssigner final
{
public:
    using DeclarationConsumer = std::function<void (const Profiler::AllocationGroup &, Event)>;

    // NOTE: Implements increment-from-zero mechanism, expected by Recording.

    GroupUIDAssigner () noexcept = default;

    [[nodiscard]] GroupUID GetUID (const Profiler::AllocationGroup &_group) const noexcept;

    GroupUID GetOrAssignUID (const Profiler::AllocationGroup &_group,
                             const DeclarationConsumer &_declarationConsumer) noexcept;

    void ImportCapture (const Profiler::CapturedAllocationGroup &_captured,
                        const DeclarationConsumer &_declarationConsumer) noexcept;

    void Clear () noexcept;

private:
    GroupUID counter = 0u;
    Container::HashMap<Profiler::AllocationGroup, GroupUID> ids {Constants::AllocationGroup ()};
};

Event ConvertEvent (GroupUID _predictedUid, const Profiler::Event &_source) noexcept;
} // namespace Emergence::Memory::Recording
