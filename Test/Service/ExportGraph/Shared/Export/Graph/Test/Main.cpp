#include <Export/Graph/Test/Export.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool EXPORT_TEST_INCLUDE_MARKER = Emergence::Export::Graph::Test::ExportTestIncludeMarker ();
