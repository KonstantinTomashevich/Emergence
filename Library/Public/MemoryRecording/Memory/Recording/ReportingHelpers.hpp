#pragma once

#include <functional>

#include <Container/HashMap.hpp>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/Constants.hpp>
#include <Memory/Recording/Event.hpp>

namespace Emergence::Memory::Recording
{
/// \brief Encapsulates shared logic for GroupUID and EventType::DECLARE_GROUP events generation.
class GroupUIDAssigner final
{
public:
    /// \brief Consumer for created EventType::DECLARE_GROUP events.
    using DeclarationConsumer = std::function<void (Event)>;

    GroupUIDAssigner () noexcept = default;

    /// \return Group UID, if group is declared, `MISSING_GROUP_ID` otherwise.
    [[nodiscard]] GroupUID GetUID (const Profiler::AllocationGroup &_group) const noexcept;

    /// \details If group is not declared, declares it and passes declaration event to consumer.
    ///          If parent group has no UID, assigns it before assigning UID to child group.
    ///          Parent UID check is recursive.
    GroupUID GetOrAssignUID (const Profiler::AllocationGroup &_group,
                             const DeclarationConsumer &_declarationConsumer) noexcept;

    /// \brief Creates group declaration events for all groups in captured hierarchy.
    void ImportCapture (const Profiler::CapturedAllocationGroup &_captured,
                        const DeclarationConsumer &_declarationConsumer) noexcept;

    /// \brief Resets assigner to initial state.
    void Clear () noexcept;

private:
    GroupUID counter = 0u;
    Container::HashMap<Profiler::AllocationGroup, GroupUID> ids {Constants::AllocationGroup ()};
};

/// \brief Converts MemoryProfiler event into MemoryRecording event.
/// \param _groupUID UID for Profiler::Event::group, obtained by external code. For example, using GroupUIDAssigner.
Event ConvertEvent (GroupUID _groupUID, const Profiler::Event &_source) noexcept;
} // namespace Emergence::Memory::Recording
