#include <SDL3/SDL_main.h>

#include <Application/Application.hpp>

#include <CPU/Profiler.hpp>

int main (int /*unused*/, char ** /*unused*/)
{
    Emergence::CPU::Profiler::SetThreadName ("Main");
    Application {}.Run ();
    return 0;
}
