#include <RecordCollection/Test/Collection.hpp>
#include <RecordCollection/Test/LinearRepresentation.hpp>
#include <RecordCollection/Test/PointRepresentation.hpp>
#include <RecordCollection/Test/Visualization.hpp>
#include <RecordCollection/Test/VolumetricRepresentation.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool COLLECTION_TEST_INCLUDE_MARKER = Emergence::RecordCollection::Test::CollectionTestIncludeMarker ();

static const bool LINEAR_REPRESENTATION_TEST_INCLUDE_MARKER =
    Emergence::RecordCollection::Test::LinearRepresentationTestIncludeMarker ();

static const bool POINT_REPRESENTATION_TEST_INCLUDE_MARKER =
    Emergence::RecordCollection::Test::PointRepresentationTestIncludeMarker ();

static const bool VISUALIZATION_TEST_INCLUDE_MARKER =
    Emergence::RecordCollection::Test::VisualizationTestIncludeMarker ();

static const bool VOLUMETRIC_REPRESENTATION_TEST_INCLUDE_MARKER =
    Emergence::RecordCollection::Test::VolumetricRepresentationTestIncludeMarker ();
