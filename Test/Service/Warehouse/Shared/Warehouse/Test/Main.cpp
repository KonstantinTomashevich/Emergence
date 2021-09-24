#include <Testing/SetupMain.hpp>

#include <Warehouse/Test/CursorReference.hpp>
#include <Warehouse/Test/PreparedQueryReference.hpp>
#include <Warehouse/Test/QueryAPI.hpp>
#include <Warehouse/Test/Visualization.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool CURSOR_REFERENCE_TEST_INCLUDE_MARKER =
    Emergence::Warehouse::Test::CursorReferenceTestIncludeMarker ();

static const bool PREPARED_QUERY_REFERENCE_TEST_INCLUDE_MARKER =
    Emergence::Warehouse::Test::PreparedQueryReferenceTestIncludeMarker ();

static const bool QUERY_API_TEST_INCLUDE_MARKER = Emergence::Warehouse::Test::QueryAPITestIncludeMarker ();

static const bool VISUALIZATION_TEST_INCLUDE_MARKER = Emergence::Warehouse::Test::VisualizationTestIncludeMarker ();
