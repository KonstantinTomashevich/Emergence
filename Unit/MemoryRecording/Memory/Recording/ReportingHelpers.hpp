#pragma once

#include <MemoryRecordingApi.hpp>

#include <functional>

#include <Container/HashMap.hpp>

#include <Memory/Profiler/Capture.hpp>

#include <Memory/Recording/Constants.hpp>
#include <Memory/Recording/Event.hpp>

namespace Emergence::Memory::Recording
{
/// \brief Encapsulates shared logic for GroupUID and EventType::DECLARE_GROUP events generation.
class MemoryRecordingApi GroupUIDAssigner final
{
public:
    /// \brief Consumer for created EventType::DECLARE_GROUP events.
    using DeclarationConsumer = std::function<void (Event)>;

    GroupUIDAssigner () noexcept = default;

    /// \return Group UID, if group is declared, `MISSING_GROUP_ID` otherwise.
    [[nodiscard]] GroupUID GetUID (const Profiler::AllocationGroup &_group) const noexcept;

    /// \details Ensures that given group and its parents (recursively) are declared. If any of them is not,
    ///          assigns UIDs and generated EventType::DECLARE_GROUP events for undeclared groups.
    /// \return UID of given group.
    GroupUID GetOrAssignUID (const Profiler::AllocationGroup &_group,
                             const DeclarationConsumer &_declarationConsumer) noexcept;

    /// \brief Assigns UIDs and creates declaration events for all groups in captured hierarchy.
    void ImportCapture (const Profiler::CapturedAllocationGroup &_captured,
                        const DeclarationConsumer &_declarationConsumer) noexcept;

    /// \brief Resets assigner to initial state.
    void Clear () noexcept;

private:
    GroupUID counter = 0u;
    Container::HashMap<Profiler::AllocationGroup, GroupUID> uids {Constants::AllocationGroup ()};
};

/// \brief Converts MemoryProfiler event into MemoryRecording event.
/// \param _groupUID UID for Profiler::Event::group, obtained by external code. For example, using GroupUIDAssigner.
MemoryRecordingApi Event ConvertEvent (GroupUID _groupUID, const Profiler::Event &_source) noexcept;
} // namespace Emergence::Memory::Recording
