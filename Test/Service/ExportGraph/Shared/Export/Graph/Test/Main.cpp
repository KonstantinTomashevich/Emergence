#include <Export/Graph/Test/Export.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool exportTestIncludeMarker = Emergence::Export::Graph::Test::ExportTestIncludeMarker ();
