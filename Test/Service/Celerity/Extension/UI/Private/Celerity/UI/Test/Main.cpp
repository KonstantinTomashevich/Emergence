#include <Celerity/UI/Test/Functional.hpp>
#include <Celerity/UI/Test/Visual.hpp>

#include <cstring>

#if defined(__unix__)
#    include <SDL2/SDL.h>
#else
#    include <SDL.h>
#endif

#include <Testing/SetupMain.hpp>

static const bool UI_FUNCTIONAL_TEST_INCLUDE_MARKER = Emergence::Celerity::Test::FunctionalTestIncludeMarker ();
static const bool UI_VISUAL_TEST_INCLUDE_MARKER = Emergence::Celerity::Test::VisualTestIncludeMarker ();
