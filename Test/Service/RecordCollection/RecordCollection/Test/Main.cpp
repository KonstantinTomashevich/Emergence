#include <RecordCollection/Test/Collection.hpp>
#include <RecordCollection/Test/LinearRepresentation.hpp>
#include <RecordCollection/Test/PointRepresentation.hpp>
#include <RecordCollection/Test/VolumetricRepresentation.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool collectionTestIncludeMarker = Emergence::RecordCollection::Test::CollectionTestIncludeMarker ();

static const bool linearRepresentationTestIncludeMarker =
    Emergence::RecordCollection::Test::LinearRepresentationTestIncludeMarker ();

static const bool pointRepresentationTestIncludeMarker =
    Emergence::RecordCollection::Test::PointRepresentationTestIncludeMarker ();

static const bool volumetricRepresentationTestIncludeMarker =
    Emergence::RecordCollection::Test::VolumetricRepresentationTestIncludeMarker ();
