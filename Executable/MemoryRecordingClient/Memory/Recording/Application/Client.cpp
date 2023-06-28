#include <Assert/Assert.hpp>

#include <cstdio>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <Memory/Recording/Application/Client.hpp>

namespace Emergence::Memory::Recording::Application
{
Client::Client () noexcept
{
    SetIsAssertInteractive (true);
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf ("SDL Error: %s\n", SDL_GetError ());
        initializedSuccessfully = false;
        return;
    }

    auto windowFlags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    // TODO: Currently we hardcode window initial size for simplicity.
    window = SDL_CreateWindow ("Memory Recording Client", 1280, 720, windowFlags);

    renderer = SDL_CreateRenderer (window, nullptr, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
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
    ImGui_ImplSDL3_InitForSDLRenderer (window, renderer);
    ImGui_ImplSDLRenderer3_Init (renderer);
}

Client::~Client () noexcept
{
    ImGui_ImplSDLRenderer3_Shutdown ();
    ImGui_ImplSDL3_Shutdown ();
    ImGui::DestroyContext ();

    SDL_DestroyRenderer (renderer);
    SDL_DestroyWindow (window);
    SDL_Quit ();
}

TrackHolder &Client::GetTrackHolder () noexcept
{
    return trackHolder;
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
            ImGui_ImplSDL3_ProcessEvent (&event);
            if (event.type == SDL_EVENT_QUIT)
            {
                done = true;
            }

            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID (window))
            {
                done = true;
            }
        }

        trackHolder.Update ();
        ImGui_ImplSDLRenderer3_NewFrame ();
        ImGui_ImplSDL3_NewFrame ();

        ImGui::NewFrame ();
        ui.Render (*this);
        ImGui::Render ();

        SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
        SDL_RenderClear (renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData (ImGui::GetDrawData ());
        SDL_RenderPresent (renderer);
    }

    return 0;
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
