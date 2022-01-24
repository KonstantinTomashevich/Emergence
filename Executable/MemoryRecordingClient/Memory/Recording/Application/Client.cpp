#include <cstdio>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include <Memory/Recording/Application/Client.hpp>

namespace Emergence::Memory::Recording::Application
{
Client::Client () noexcept
{
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf ("SDL Error: %s\n", SDL_GetError ());
        initializedSuccessfully = false;
        return;
    }

    auto windowFlags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    // TODO: Currently we hardcode window initial size for simplicity.
    window = SDL_CreateWindow ("Memory Recording Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
                               windowFlags);

    renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        SDL_Log ("Error creating SDL_Renderer!");
        initializedSuccessfully = false;
        return;
    }

    IMGUI_CHECKVERSION ();
    ImGui::CreateContext ();
    ImGui::GetIO ().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark ();
    ImGui_ImplSDL2_InitForSDLRenderer (window);
    ImGui_ImplSDLRenderer_Init (renderer);
}

Client::~Client () noexcept
{
    ImGui_ImplSDLRenderer_Shutdown ();
    ImGui_ImplSDL2_Shutdown ();
    ImGui::DestroyContext ();

    SDL_DestroyRenderer (renderer);
    SDL_DestroyWindow (window);
    SDL_Quit ();
}

const TrackHolder &Client::GetTrackHolder () const noexcept
{
    return trackHolder;
}

void Client::OpenTrack (const char *_fileName) noexcept
{
    trackHolder.Open (_fileName);
}

float Client::GetSelectedTimeS () const noexcept
{
    return GetEventTime (trackHolder.GetTrack ().EventCurrent ());
}

void Client::SelectTime (float _seconds) noexcept
{
    requestedTimeS = _seconds;
}

int Client::Run () noexcept
{
    if (!initializedSuccessfully)
    {
        return -1;
    }

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent (&event))
        {
            ImGui_ImplSDL2_ProcessEvent (&event);
            if (event.type == SDL_QUIT)
            {
                done = true;
            }

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID (window))
            {
                done = true;
            }
        }

        trackHolder.Update ();
        UpdateSelectedTime ();

        ImGui_ImplSDLRenderer_NewFrame ();
        ImGui_ImplSDL2_NewFrame (window);

        ImGui::NewFrame ();
        ui.Render (*this);
        ImGui::Render ();

        SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
        SDL_RenderClear (renderer);
        ImGui_ImplSDLRenderer_RenderDrawData (ImGui::GetDrawData ());
        SDL_RenderPresent (renderer);
    }

    return 0;
}

void Client::UpdateSelectedTime () noexcept
{
    if (requestedTimeS)
    {
        using namespace std::chrono_literals;
        using Clock = std::chrono::high_resolution_clock;
        static const Clock ::duration timeout = 20ms;

        const Clock::time_point updateStart = Clock::now ();
        const Track &track = trackHolder.GetTrack ();
        Track::EventIterator currentEvent = track.EventCurrent ();
        const float currentTimeS = GetEventTime (currentEvent);

        const bool goingForward = requestedTimeS.value () > currentTimeS;

        while (true)
        {
            if (goingForward)
            {
                ++currentEvent;
                if (GetEventTime (currentEvent) > requestedTimeS.value () || !trackHolder.MoveToNextEvent ())
                {
                    requestedTimeS.reset ();
                    break;
                }
            }
            else
            {
                --currentEvent;
                const bool lastUndo = GetEventTime (currentEvent) < requestedTimeS.value ();

                if (!trackHolder.MoveToPreviousEvent () || lastUndo)
                {
                    requestedTimeS.reset ();
                    break;
                }
            }

            if (Clock::now () - updateStart > timeout)
            {
                break;
            }
        }
    }
}

float Client::GetEventTime (const Track::EventIterator &_iterator) const noexcept
{
    const Track &track = trackHolder.GetTrack ();
    return _iterator == track.EventEnd () ? 0.0f : static_cast<float> ((*_iterator)->timeNs) * 1e-9f;
}

int Main (int /*unused*/, char ** /*unused*/)
{
    return Emergence::Memory::Recording::Application::Client {}.Run ();
}
} // namespace Emergence::Memory::Recording::Application

int main (int _argc, char **_argv)
{
    return Emergence::Memory::Recording::Application::Main (_argc, _argv);
}
