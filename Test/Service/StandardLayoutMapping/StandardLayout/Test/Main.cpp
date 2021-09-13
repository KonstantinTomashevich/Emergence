#include <StandardLayout/Test/MappingBuilder.hpp>
#include <StandardLayout/Test/Visualization.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool mappingBuilderTestIncludeMarker = Emergence::StandardLayout::Test::MappingBuilderTestIncludeMarker ();

static const bool visualizationTestIncludeMarker = Emergence::StandardLayout::Test::VisualizationTestIncludeMarker ();
