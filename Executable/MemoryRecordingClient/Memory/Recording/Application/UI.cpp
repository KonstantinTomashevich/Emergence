#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#include <imgui.h>

#include <Container/StringBuilder.hpp>

#include <Memory/Recording/Application/Client.hpp>
#include <Memory/Recording/Application/UI.hpp>

namespace Emergence::Memory::Recording::Application
{
void UI::Render (Client &_client) noexcept
{
    ImGui::SetNextWindowSize ({ImGui::GetIO ().DisplaySize.x, ImGui::GetIO ().DisplaySize.y});
    ImGui::SetNextWindowPos ({0.0f, 0.0f});

    ImGui::Begin ("Memory Recording Client", nullptr,
                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                      ImGuiWindowFlags_NoTitleBar);

    DrawOpenTrackMenu (_client);
    DrawFlameGraph (_client);
    DrawTimeline (_client);

    ImGui::End ();
}

uintptr_t UI::ExtractGroupColor (const RecordedAllocationGroup *_group,
                                 float _minBrightness,
                                 float _maxBrightness) noexcept
{
    // Unique string hash values are not very distinct, therefore we sacrifice performance here for better colors.
    const size_t hash = *_group->GetId () ? std::hash<std::string_view> {}(*_group->GetId ()) : 0u;

    constexpr size_t R_MASK = 255u << 16u;
    const size_t r = (hash & R_MASK) >> 16u;

    constexpr size_t G_MASK = 255u << 8u;
    const size_t g = (hash & G_MASK) >> 8u;

    constexpr size_t B_MASK = 255u << 0u;
    const size_t b = (hash & B_MASK) >> 0u;

    auto applyBrightness = [_minBrightness, _maxBrightness] (size_t _value)
    {
        return static_cast<size_t> (_minBrightness * 255.0f +
                                    static_cast<float> (_value) * (_maxBrightness - _minBrightness));
    };

    return IM_COL32 (applyBrightness (r), applyBrightness (g), applyBrightness (b), 255);
}

std::pair<ImVec2, ImVec2> UI::GetWindowRect () noexcept
{
    const ImVec2 min = {ImGui::GetWindowPos ().x, ImGui::GetWindowPos ().y};
    const ImVec2 max = {min.x + ImGui::GetWindowSize ().x, min.y + ImGui::GetWindowSize ().y};
    return {min, max};
}

bool UI::IsMouseInside (const ImVec2 &_min, const ImVec2 &_max) noexcept
{
    const ImVec2 mousePosition = ImGui::GetMousePos ();
    return !(mousePosition.x < _min.x || mousePosition.x > _max.x || mousePosition.y < _min.y ||
             mousePosition.y > _max.y);
}

bool UI::IsMouseInsideCurrentWindow () noexcept
{
    auto [min, max] = GetWindowRect ();
    return IsMouseInside (min, max);
}

void UI::EnableScrollingThroughDragging () noexcept
{
    if (IsMouseInsideCurrentWindow () && ImGui::IsMouseDragging (ImGuiMouseButton_Left))
    {
        ImVec2 delta = ImGui::GetMouseDragDelta (ImGuiMouseButton_Left);
        ImGui::ResetMouseDragDelta (ImGuiMouseButton_Left);
        ImGui::SetScrollX (std::clamp (ImGui::GetScrollX () - delta.x, 0.0f, ImGui::GetScrollMaxX ()));
        ImGui::SetScrollY (std::clamp (ImGui::GetScrollY () - delta.y, 0.0f, ImGui::GetScrollMaxY ()));
    }
}

void UI::EnableHorizontalScalingThroughWheel (float &_scale, float _min, float _max) noexcept
{
    constexpr float RELATIVE_SPEED = 0.1f;
    if (IsMouseInsideCurrentWindow () && ImGui::GetIO ().MouseWheel != 0.0f)
    {
        const float previousScale = _scale;
        _scale = std::clamp (_scale + _scale * ImGui::GetIO ().MouseWheel * RELATIVE_SPEED, _min, _max);
        const float multiplier = _scale / previousScale;

        const float mouseLocalX = ImGui::GetMousePos ().x - ImGui::GetWindowPos ().x - ImGui::GetCursorStartPos ().x;
        const float mouseViewportX = mouseLocalX - ImGui::GetScrollX ();
        ImGui::SetScrollX (mouseLocalX * multiplier - mouseViewportX);
    }
}

void UI::DrawOpenTrackMenu (Client &_client) noexcept
{
    ImGui::TextUnformatted ("Open Track Menu");
    if (ImGui::InputText ("Input track file name", filePathBuffer.data (), filePathBuffer.size (),
                          ImGuiInputTextFlags_EnterReturnsTrue))
    {
        _client.OpenTrack (filePathBuffer.data ());
    }

    if (_client.GetTrackHolder ().IsLoading ())
    {
        ImGui::SameLine ();
        ImGui::TextUnformatted ("Loading...");
    }

    ImGui::Separator ();
}

void UI::DrawFlameGraph (Client &_client) noexcept
{
    ImVec2 contentMax = ImGui::GetWindowContentRegionMax ();
    ImGui::TextUnformatted ("Flame Graph");

    ImGui::BeginChild ("Flame Graph", {contentMax.x, contentMax.y * 0.5f}, true,
                       ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (const RecordedAllocationGroup *root = _client.GetTrackHolder ().GetTrack ().Root ())
    {
        const ImVec2 drawZoneStart = ImGui::GetCursorScreenPos ();
        const float drawZoneWidth = ImGui::GetContentRegionAvail ().x;

        float yEnd = DrawFlameGraphNode (root, ImGui::GetWindowDrawList (), drawZoneStart.x, drawZoneStart.y,
                                         drawZoneStart.x + drawZoneWidth * flameGraphHorizontalScale);

        ImGui::Dummy ({drawZoneWidth * flameGraphHorizontalScale, yEnd - drawZoneStart.y});
    }

    EnableHorizontalScalingThroughWheel (flameGraphHorizontalScale, 1.0f, 2000.0f);
    EnableScrollingThroughDragging ();
    ImGui::EndChild ();
}

float UI::DrawFlameGraphNode (const RecordedAllocationGroup *_group,
                              ImDrawList *_drawList,
                              float _levelXStart,
                              float _levelYStart,
                              float _levelXEnd) const noexcept
{
    constexpr float LEVEL_HEIGHT = 50.0f;

    const float currentLevelWidth = _levelXEnd - _levelXStart;
    const float acquiredWidth =
        static_cast<float> (_group->GetAcquired ()) * currentLevelWidth / static_cast<float> (_group->GetTotal ());

    const ImVec2 acquiredRectMin {_levelXStart, _levelYStart};
    const ImVec2 acquiredRectMax {_levelXStart + acquiredWidth, _levelYStart + LEVEL_HEIGHT};

    const ImVec2 reservedRectMin {acquiredRectMax.x, _levelYStart};
    const ImVec2 reservedRectMax {_levelXEnd, _levelYStart + LEVEL_HEIGHT};

    // Early culling of drawables, that would otherwise be clipped away.
    if (ImGui::IsRectVisible (acquiredRectMin, reservedRectMax))
    {
        _drawList->AddRectFilled (acquiredRectMin, acquiredRectMax, ExtractGroupColor (_group, 0.5f, 0.8f));
        _drawList->AddRectFilled (reservedRectMin, reservedRectMax, ExtractGroupColor (_group, 0.2f, 0.5f));
        _drawList->AddRect (acquiredRectMin, reservedRectMax, IM_COL32 (0, 0, 0, 255));

        const char *idString = *_group->GetId () ? *_group->GetId () : "Root";
        const auto usagePercent = static_cast<uint32_t> (static_cast<float> (_group->GetAcquired ()) * 100.0f /
                                                         static_cast<float> (_group->GetTotal ()));

        const char *info = EMERGENCE_BUILD_STRING (idString, " ", _group->GetAcquired (), "/", _group->GetTotal (),
                                                   " (", usagePercent, "%)");

        constexpr float TEXT_OFFSET_X = 10.0f;
        ImVec4 textClippingRect {acquiredRectMin.x, acquiredRectMin.y, reservedRectMax.x, reservedRectMax.y};
        _drawList->AddText (ImGui::GetFont (), ImGui::GetFontSize (), {_levelXStart + TEXT_OFFSET_X, _levelYStart},
                            IM_COL32 (255, 255, 255, 255), info, nullptr, 0.0f, &textClippingRect);

        if (IsMouseInside (acquiredRectMin, reservedRectMax))
        {
            ImGui::SetTooltip ("%s", info);
        }

        // TODO: Highlight selected.
    }

    float graphYEnd = _levelYStart + LEVEL_HEIGHT;
    float nextLevelXStart = _levelXStart;

    for (auto iterator = _group->BeginChildren (); iterator != _group->EndChildren (); ++iterator)
    {
        const RecordedAllocationGroup *next = *iterator;
        if (next->GetTotal () != 0u)
        {
            const float nextLevelWidth =
                static_cast<float> (next->GetTotal ()) * currentLevelWidth / static_cast<float> (_group->GetTotal ());
            const float nextLevelXEnd = nextLevelXStart + nextLevelWidth;

            graphYEnd = std::max (graphYEnd, DrawFlameGraphNode (next, _drawList, nextLevelXStart,
                                                                 _levelYStart + LEVEL_HEIGHT, nextLevelXEnd));
            nextLevelXStart = nextLevelXEnd;
        }
    }

    return graphYEnd;
}

void UI::DrawTimeline ([[maybe_unused]] Client &_client) noexcept
{
    ImVec2 contentMax = ImGui::GetWindowContentRegionMax ();
    ImGui::TextUnformatted ("Timeline");
    ImGui::BeginChild ("Timeline", {contentMax.x, contentMax.y * 0.15f}, true,
                       ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    const Track &track = _client.GetTrackHolder ().GetTrack ();
    if (track.EventBegin () != track.EventEnd ())
    {
        // TODO: Watch out for precision bugs.
        const float timeBeginS = static_cast<float> ((*track.EventBegin ())->timeNs) * 1e-9f;
        const float timeEndS = static_cast<float> ((*--track.EventEnd ())->timeNs) * 1e-9f;
        const float timeDurationS = timeEndS - timeBeginS;

        const ImVec2 drawZoneStart = ImGui::GetCursorScreenPos ();
        const ImVec2 drawZoneFrameStart = {drawZoneStart.x + ImGui::GetScrollX (), drawZoneStart.y};
        const ImVec2 drawZoneVisibleSize = ImGui::GetContentRegionAvail ();
        const float drawZoneFullWidth = drawZoneVisibleSize.x * timelineScale;

        const float timeFrameBeginS = std::lerp (timeBeginS, timeEndS, ImGui::GetScrollX () / drawZoneFullWidth);
        const float timeFrameEndS =
            std::lerp (timeBeginS, timeEndS, (ImGui::GetScrollX () + drawZoneVisibleSize.x) / drawZoneFullWidth);
        const float pixelsPerSecond = drawZoneFullWidth / timeDurationS;

        constexpr uint32_t LINES_COLOR = IM_COL32 (150, 150, 150, 255);
        constexpr uint32_t TEXT_COLOR = IM_COL32 (220, 220, 220, 255);
        constexpr uint32_t MARKER_COLOR = IM_COL32 (255, 180, 0, 255);
        constexpr uint32_t SELECTION_COLOR = IM_COL32 (200, 0, 0, 255);

        ImDrawList *drawList = ImGui::GetWindowDrawList ();
        drawList->AddLine (
            {drawZoneFrameStart.x, drawZoneFrameStart.y + drawZoneVisibleSize.y * 0.5f},
            {drawZoneFrameStart.x + drawZoneVisibleSize.x, drawZoneFrameStart.y + drawZoneVisibleSize.y * 0.5f},
            LINES_COLOR);

        auto secondToX = [&drawZoneStart, timeBeginS, pixelsPerSecond] (float _second)
        {
            return drawZoneStart.x + (_second - timeBeginS) * pixelsPerSecond;
        };

        Container::StringBuilder builder;
        const auto firstSecond = static_cast<uint64_t> (floor (timeFrameBeginS));
        const auto lastSecond = static_cast<uint64_t> (ceil (timeFrameEndS));

        // TODO: Think about refactoring this cycle.
        for (size_t second = firstSecond; second <= lastSecond; ++second)
        {
            const bool isMinute = second % 60u == 0u;

            // If there is huge amount of seconds on screen, draw only minutes.
            if (pixelsPerSecond < 10.0f && !isMinute)
            {
                continue;
            }

            const float x = secondToX (static_cast<float> (second));
            const float lineSize = isMinute ? 0.8f : 0.6f;

            drawList->AddLine ({x, drawZoneStart.y + drawZoneVisibleSize.y * (0.5f - lineSize * 0.5f)},
                               {x, drawZoneStart.y + drawZoneVisibleSize.y * (0.5f + lineSize * 0.5f)}, LINES_COLOR);

            if (pixelsPerSecond >= 30.0f || isMinute)
            {
                drawList->AddText ({x, drawZoneStart.y + drawZoneVisibleSize.y * (0.5f + lineSize * 0.5f)}, TEXT_COLOR,
                                   builder.Reset ().Append (second / 60u, "m", second % 60u, 's').Get ());
            }

            // Draw 100ms guidelines.
            if (pixelsPerSecond >= 100.0f)
            {
                for (size_t lineIndex = 1u; lineIndex < 10u; ++lineIndex)
                {
                    const float x = secondToX (static_cast<float> (second) + static_cast<float> (lineIndex) * 0.1f);
                    drawList->AddLine ({x, drawZoneStart.y + drawZoneVisibleSize.y * 0.35f},
                                       {x, drawZoneStart.y + drawZoneVisibleSize.y * 0.65f}, LINES_COLOR);
                }
            }

            // Draw 10ms guidelines.
            if (pixelsPerSecond >= 1000.0f)
            {
                for (size_t lineIndex = 1u; lineIndex < 100u; ++lineIndex)
                {
                    const float x = secondToX (static_cast<float> (second) + static_cast<float> (lineIndex) * 0.01f);
                    drawList->AddLine ({x, drawZoneStart.y + drawZoneVisibleSize.y * 0.45f},
                                       {x, drawZoneStart.y + drawZoneVisibleSize.y * 0.55f}, LINES_COLOR);
                }
            }
        }

        // Draw visible markers.
        auto [firstMarker, lastMarker] = _client.GetTrackHolder ().GetMarkersInBounds (timeFrameBeginS, timeFrameEndS);

        for (auto iterator = firstMarker; iterator != lastMarker; ++iterator)
        {
            const double frequencyS = _client.GetTrackHolder ().GetMarkerFrequency ((**iterator)->markerId);

            // Cull out markers with too high frequency for current scale.
            if (pixelsPerSecond > 1000.0f || (pixelsPerSecond > 100.0f && frequencyS > 0.1) || frequencyS > 1.0)
            {
                const float markerSecond = static_cast<float> ((**iterator)->timeNs) * 1e-9f;
                const float x = secondToX (markerSecond);

                constexpr float MARKER_RADIUS = 5.0f;
                drawList->AddCircleFilled ({x, drawZoneStart.y + drawZoneVisibleSize.y * 0.5f}, MARKER_RADIUS,
                                           MARKER_COLOR);

                const ImVec2 tooltipMin {x - MARKER_RADIUS,
                                         drawZoneStart.y + drawZoneVisibleSize.y * 0.5f - MARKER_RADIUS};

                const ImVec2 tooltipMax {x + MARKER_RADIUS,
                                         drawZoneStart.y + drawZoneVisibleSize.y * 0.5f + MARKER_RADIUS};

                if (IsMouseInside (tooltipMin, tooltipMax))
                {
                    ImGui::SetTooltip ("%s", *(**iterator)->markerId);
                }
            }
        }

        // Mark current selected time.
        float selectedX = secondToX (_client.GetSelectedTimeS ());
        drawList->AddLine ({selectedX, drawZoneStart.y}, {selectedX, drawZoneStart.y + drawZoneVisibleSize.y},
                           SELECTION_COLOR);

        if (IsMouseInsideCurrentWindow () && ImGui::IsMouseDoubleClicked (ImGuiMouseButton_Left))
        {
            float localX = ImGui::GetMousePos ().x - drawZoneStart.x;
            _client.SelectTime (timeBeginS + localX * timeDurationS / drawZoneFullWidth);
        }

        ImGui::Dummy ({drawZoneFullWidth, drawZoneVisibleSize.y});
    }

    EnableHorizontalScalingThroughWheel (timelineScale, 1.0f, 2000.0f);
    EnableScrollingThroughDragging ();
    ImGui::EndChild ();
}
} // namespace Emergence::Memory::Recording::Application
