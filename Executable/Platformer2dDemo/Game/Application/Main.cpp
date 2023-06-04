#if defined(__unix__)
#    include <SDL2/SDL.h>
#else
#    include <SDL.h>
#endif

#include <Application/Application.hpp>

#include <CPU/Profiler.hpp>

int main (int /*unused*/, char ** /*unused*/)
{
    Emergence::CPU::Profiler::SetThreadName ("Main");
    Application {}.Run ();
    return 0;
}
