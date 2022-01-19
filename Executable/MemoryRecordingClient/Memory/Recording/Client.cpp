#include <cstdio>

#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

int main (int /*unused*/, char ** /*unused*/)
{
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf ("SDL Error: %s\n", SDL_GetError ());
        return -1;
    }

    auto windowFlags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow ("Memory Recording Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                           1280, 720, windowFlags);

    SDL_Renderer *renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        SDL_Log ("Error creating SDL_Renderer!");
        return false;
    }

    IMGUI_CHECKVERSION ();
    ImGui::CreateContext ();
    ImGuiIO &io = ImGui::GetIO ();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark ();
    ImGui_ImplSDL2_InitForSDLRenderer (window);
    ImGui_ImplSDLRenderer_Init (renderer);

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

        ImGui_ImplSDLRenderer_NewFrame ();
        ImGui_ImplSDL2_NewFrame (window);
        ImGui::NewFrame ();

        // ImGUI example.
        {
            static float real = 0.0f;
            static int counter = 0;
            static bool flag = false;

            ImGui::Begin ("Hello, world!");

            ImGui::Text ("This is some useful text.");
            ImGui::Checkbox ("Flag", &flag);
            ImGui::SliderFloat ("Real", &real, 0.0f, 1.0f);

            if (ImGui::Button ("Button"))
            {
                counter++;
            }

            ImGui::SameLine ();
            ImGui::Text ("counter = %d", counter);

            ImGui::Text ("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO ().Framerate,
                         ImGui::GetIO ().Framerate);
            ImGui::End ();
        }

        ImGui::Render ();
        SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
        SDL_RenderClear (renderer);
        ImGui_ImplSDLRenderer_RenderDrawData (ImGui::GetDrawData ());
        SDL_RenderPresent (renderer);
    }

    ImGui_ImplSDLRenderer_Shutdown ();
    ImGui_ImplSDL2_Shutdown ();
    ImGui::DestroyContext ();

    SDL_DestroyRenderer (renderer);
    SDL_DestroyWindow (window);
    SDL_Quit ();

    return 0;
}
