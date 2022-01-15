#pragma once

#include <Memory/Recording/Event.hpp>

namespace Emergence::Memory::Recording
{
class Recording;

class DeserializerBase
{
protected:
    DeserializerBase () noexcept = default;

    void Begin (Recording *_recording) noexcept;

    void ReportEvent (const Event &_event) noexcept;

    void End () noexcept;

    [[nodiscard]] Recording *GetRecording () const noexcept;

private:
    Recording *recording = nullptr;
};
} // namespace Emergence::Memory::Recording
