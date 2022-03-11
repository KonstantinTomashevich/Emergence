#include <Celerity/Test/EventRouting.hpp>
#include <Celerity/Test/EventTriggering.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool CELERITY_EVENT_ROUTING_TEST_INCLUDE_MARKER =
    Emergence::Celerity::Test::EventRoutingTestIncludeMarker ();

static const bool CELERITY_EVENT_TRIGGERING_TEST_INCLUDE_MARKER =
    Emergence::Celerity::Test::EventTriggeringTestIncludeMarker ();