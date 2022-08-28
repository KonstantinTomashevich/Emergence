#include <StandardLayout/Test/ConditionalFieldIteration.hpp>
#include <StandardLayout/Test/MappingBuilder.hpp>
#include <StandardLayout/Test/Patch.hpp>
#include <StandardLayout/Test/PatchBuilder.hpp>
#include <StandardLayout/Test/Visualization.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool CONDITIONAL_FIELD_ITERATION_TEST_INCLUDE_MARKER =
    Emergence::StandardLayout::Test::ConditionalFieldIterationTestIncludeMarker ();

static const bool MAPPING_BUILDER_TEST_INCLUDE_MARKER =
    Emergence::StandardLayout::Test::MappingBuilderTestIncludeMarker ();

static const bool PATCH_TEST_INCLUDE_MARKER = Emergence::StandardLayout::Test::PatchTestIncludeMarker ();

static const bool PATCH_BUILDER_TEST_INCLUDE_MARKER = Emergence::StandardLayout::Test::PatchBuilderTestIncludeMarker ();

static const bool VISUALIZATION_TEST_INCLUDE_MARKER =
    Emergence::StandardLayout::Test::VisualizationTestIncludeMarker ();
