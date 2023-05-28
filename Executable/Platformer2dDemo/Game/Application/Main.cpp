#if defined(__unix__)
#    include <SDL2/SDL.h>
#else
#    include <SDL.h>
#endif

#include <Application/Application.hpp>

int main (int /*unused*/, char ** /*unused*/)
{
    Application {}.Run ();
    return 0;
}
