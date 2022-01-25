#pragma once

#include <array>

namespace Emergence::Memory::Recording::Application
{
class UI final
{
public:
    void Render (class Client &_client) noexcept;

private:
    static uint32_t ExtractGroupColor (const class RecordedAllocationGroup *_group,
                                        float _minBrightness,
                                        float _maxBrightness) noexcept;

    static std::pair<ImVec2, ImVec2> GetWindowRect () noexcept;

    static bool IsMouseInside (const ImVec2 &_min, const ImVec2 &_max) noexcept;

    static bool IsMouseInsideCurrentWindow () noexcept;

    static void EnableScrollingThroughDragging () noexcept;

    static void EnableHorizontalScalingThroughWheel (float &_scale, float _min, float _max) noexcept;

    static void RenderSelection (Client &_client) noexcept;

    static void RenderEventsNearby (Client &_client);

    static void RenderSelectedGroup (Client &_client);

    void RenderOpenTrackMenu (Client &_client) noexcept;

    void RenderFlameGraph (Client &_client) noexcept;

    float RenderFlameGraphNode (Client &_client,
                                const class RecordedAllocationGroup *_group,
                                struct ImDrawList *_drawList,
                                float _levelXStart,
                                float _levelYStart,
                                float _levelXEnd) const noexcept;

    void RenderTimeline (Client &_client) noexcept;

    std::array<char, 1024u> filePathBuffer {0u};

    float flameGraphHorizontalScale = 1.0f;

    float timelineScale = 1.0f;
};
} // namespace Emergence::Memory::Recording::Application
