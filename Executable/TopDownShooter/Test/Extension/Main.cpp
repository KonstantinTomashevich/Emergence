#include <Celerity/Test/EventRouting.hpp>
#include <Celerity/Test/EventTriggering.hpp>

#include <Testing/SetupMain.hpp>

#include <Transform/Test/Operation.hpp>
#include <Transform/Test/Sync.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool CELERITY_EVENT_ROUTING_TEST_INCLUDE_MARKER =
    Emergence::Celerity::Test::EventRoutingTestIncludeMarker ();

static const bool CELERITY_EVENT_TRIGGERING_TEST_INCLUDE_MARKER =
    Emergence::Celerity::Test::EventTriggeringTestIncludeMarker ();

static const bool TRANSFORM_OPERATIONS_TEST_INCLUDE_MARKER = Emergence::Transform::Test::OperationsTestIncludeMarker ();
static const bool TRANSFORM_SYNC_TEST_INCLUDE_MARKER = Emergence::Transform::Test::SyncTestIncludeMarker ();
