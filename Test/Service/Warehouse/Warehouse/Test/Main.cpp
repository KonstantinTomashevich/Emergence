#include <Testing/SetupMain.hpp>

#include <Warehouse/Test/CursorReference.hpp>
#include <Warehouse/Test/PreparedQueryReference.hpp>
#include <Warehouse/Test/QueryAPI.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool cursorReferenceTestIncludeMarker = Emergence::Warehouse::Test::CursorReferenceTestIncludeMarker ();

static const bool preparedQueryReferenceTestIncludeMarker =
    Emergence::Warehouse::Test::PreparedQueryReferenceTestIncludeMarker ();

static const bool queryAPITestIncludeMarker = Emergence::Warehouse::Test::QueryAPITestIncludeMarker ();
