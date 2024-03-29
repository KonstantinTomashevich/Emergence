#include <API/Common/BlockCast.hpp>

#include <Container/StringBuilder.hpp>

#include <Query/Test/DataTypes.hpp>

#include <RecordCollection/Test/Scenario.hpp>
#include <RecordCollection/Visualization.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory::Literals;
using namespace Emergence::RecordCollection::Test;
using namespace Emergence;

constexpr const char *COLLECTION_ROOT = VisualGraph::Common::Constants::RECORD_COLLECTION_ROOT_NODE;
constexpr const char *MAPPING_EDGE_COLOR = VisualGraph::Common::Constants::MAPPING_USAGE_COLOR;
constexpr const char *MAPPING_ROOT = VisualGraph::Common::Constants::MAPPING_ROOT_NODE;
constexpr const char *REPRESENTATION_ROOT = VisualGraph::Common::Constants::RECORD_COLLECTION_REPRESENTATION_ROOT_NODE;

static Container::StringBuilder GetPathToMappings ()
{
    using namespace VisualGraph::Common::Constants;
    return EMERGENCE_BEGIN_BUILDING_STRING (DEFAULT_ROOT_GRAPH_ID, VisualGraph::NODE_PATH_SEPARATOR, MAPPING_SUBGRAPH,
                                            VisualGraph::NODE_PATH_SEPARATOR);
}

BEGIN_SUITE (Visualization)

TEST_CASE (OneInstanceOfEachRepresentationType)
{
    const VisualGraph::Graph result = Scenario {
        Query::Test::PlayerWithBoundingBox::Reflect ().mapping,
        {
            CreateLinearRepresentation {
                "playerName", StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().player,
                                                                  Query::Test::Player::Reflect ().name)},

            CreatePointRepresentation {
                "playerAlive",
                {
                    StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().player,
                                                        Query::Test::Player::Reflect ().alive),
                }},

            CreateSignalRepresentation {
                "playerId",
                StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().player,
                                                    Query::Test::Player::Reflect ().id),
                array_cast<std::uint32_t, sizeof (std::uint64_t)> (176345u)},

            CreateVolumetricRepresentation {
                "2d",
                {{
                     -100.0f,
                     StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().boundingBox,
                                                         Query::Test::BoundingBox::Reflect ().minX),

                     100.0f,
                     StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().boundingBox,
                                                         Query::Test::BoundingBox::Reflect ().maxX),
                 },
                 {
                     -100.0f,
                     StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().boundingBox,
                                                         Query::Test::BoundingBox::Reflect ().minY),

                     100.0f,
                     StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().boundingBox,
                                                         Query::Test::BoundingBox::Reflect ().maxY),
                 }}},
        }}.ExecuteAndVisualize ();

    const Container::String mappingPath =
        GetPathToMappings ()
            .Append (Query::Test::PlayerWithBoundingBox::Reflect ().mapping.GetName (),
                     VisualGraph::NODE_PATH_SEPARATOR)
            .Get ();

    const VisualGraph::Graph expected = {
        "RecordCollection {PlayerWithBoundingBox}",
        {},
        {{"LinearRepresentation {player.name}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "player.name", MAPPING_EDGE_COLOR}}},
         {"PointRepresentation {player.alive}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "player.alive", MAPPING_EDGE_COLOR}}},
         {"SignalRepresentation {player.id = 176345}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "player.id", MAPPING_EDGE_COLOR}}},
         {"VolumetricRepresentation {{boundingBox.minX, boundingBox.maxX}, {boundingBox.minY, boundingBox.maxY}}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "boundingBox.minX", MAPPING_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "boundingBox.maxX", MAPPING_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "boundingBox.minY", MAPPING_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "boundingBox.maxY", MAPPING_EDGE_COLOR}}}},
        {{COLLECTION_ROOT, {}}},
        {{COLLECTION_ROOT, mappingPath + MAPPING_ROOT, MAPPING_EDGE_COLOR}}};

    CHECK (result == expected);
}

TEST_CASE (MultipleInstancesOfLinearRepresentation)
{
    const VisualGraph::Graph result = Scenario {
        Query::Test::Player::Reflect ().mapping,
        {
            CreateLinearRepresentation {"playerId", Query::Test::Player::Reflect ().id},
            CreateLinearRepresentation {"playerName", Query::Test::Player::Reflect ().name},
        }}.ExecuteAndVisualize ();

    const Container::String mappingPath =
        GetPathToMappings ()
            .Append (Query::Test::Player::Reflect ().mapping.GetName (), VisualGraph::NODE_PATH_SEPARATOR)
            .Get ();

    const VisualGraph::Graph expected = {"RecordCollection {Player}",
                                         {},
                                         {{"LinearRepresentation {id}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "id", MAPPING_EDGE_COLOR}}},
                                          {"LinearRepresentation {name}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "name", MAPPING_EDGE_COLOR}}}},
                                         {{COLLECTION_ROOT, {}}},
                                         {{COLLECTION_ROOT, mappingPath + MAPPING_ROOT, MAPPING_EDGE_COLOR}}};

    CHECK (result == expected);
}

TEST_CASE (MultipleInstancesOfPointRepresentation)
{
    const VisualGraph::Graph result = Scenario {
        Query::Test::Player::Reflect ().mapping,
        {
            CreatePointRepresentation {"playerIdAndName",
                                       {Query::Test::Player::Reflect ().id, Query::Test::Player::Reflect ().name}},
            CreatePointRepresentation {"playerName", {Query::Test::Player::Reflect ().name}},
        }}.ExecuteAndVisualize ();

    const Container::String mappingPath =
        GetPathToMappings ()
            .Append (Query::Test::Player::Reflect ().mapping.GetName (), VisualGraph::NODE_PATH_SEPARATOR)
            .Get ();

    const VisualGraph::Graph expected = {"RecordCollection {Player}",
                                         {},
                                         {{"PointRepresentation {id, name}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "id", MAPPING_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "name", MAPPING_EDGE_COLOR}}},
                                          {"PointRepresentation {name}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "name", MAPPING_EDGE_COLOR}}}},
                                         {{COLLECTION_ROOT, {}}},
                                         {{COLLECTION_ROOT, mappingPath + MAPPING_ROOT, MAPPING_EDGE_COLOR}}};

    CHECK (result == expected);
}

TEST_CASE (MultipleInstancesOfSignalRepresentation)
{
    const VisualGraph::Graph result = Scenario {
        Query::Test::Player::Reflect ().mapping,
        {
            CreateSignalRepresentation {"playerId", Query::Test::Player::Reflect ().id,
                                        array_cast<std::uint32_t, sizeof (std::uint64_t)> (349u)},
            CreateSignalRepresentation {"classId", Query::Test::Player::Reflect ().classId,
                                        array_cast<Memory::UniqueString, sizeof (std::uint64_t)> ("Knight"_us)},
        }}.ExecuteAndVisualize ();

    const Container::String mappingPath =
        GetPathToMappings ()
            .Append (Query::Test::Player::Reflect ().mapping.GetName (), VisualGraph::NODE_PATH_SEPARATOR)
            .Get ();
    const VisualGraph::Graph expected = {"RecordCollection {Player}",
                                         {},
                                         {{"SignalRepresentation {id = 349}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "id", MAPPING_EDGE_COLOR}}},
                                          {"SignalRepresentation {classId = Knight}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "classId", MAPPING_EDGE_COLOR}}}},
                                         {{COLLECTION_ROOT, {}}},
                                         {{COLLECTION_ROOT, mappingPath + MAPPING_ROOT, MAPPING_EDGE_COLOR}}};

    CHECK (result == expected);
}

TEST_CASE (MultipleInstancesOfVolumetricRepresentation)
{
    const Container::Vector<Query::Test::Sources::Volumetric::Dimension> dimensions2d = {
        {
            -100.0f,
            Query::Test::BoundingBox::Reflect ().minX,
            100.0f,
            Query::Test::BoundingBox::Reflect ().maxX,
        },
        {
            -100.0f,
            Query::Test::BoundingBox::Reflect ().minY,
            100.0f,
            Query::Test::BoundingBox::Reflect ().maxY,
        }};

    Container::Vector<Query::Test::Sources::Volumetric::Dimension> dimensions3d = dimensions2d;
    dimensions3d.emplace_back (Query::Test::Sources::Volumetric::Dimension {
        -100.0f,
        Query::Test::BoundingBox::Reflect ().minZ,
        100.0f,
        Query::Test::BoundingBox::Reflect ().maxZ,
    });

    const VisualGraph::Graph result = Scenario {
        Query::Test::BoundingBox::Reflect ().mapping,
        {
            CreateVolumetricRepresentation {"2d", dimensions2d},
            CreateVolumetricRepresentation {"3d", dimensions3d},
        }}.ExecuteAndVisualize ();

    const Container::String mappingPath =
        GetPathToMappings ()
            .Append (Query::Test::BoundingBox::Reflect ().mapping.GetName (), VisualGraph::NODE_PATH_SEPARATOR)
            .Get ();

    const VisualGraph::Graph expected = {"RecordCollection {BoundingBox}",
                                         {},
                                         {{"VolumetricRepresentation {{minX, maxX}, {minY, maxY}}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "minX", MAPPING_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "maxX", MAPPING_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "minY", MAPPING_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "maxY", MAPPING_EDGE_COLOR}}},
                                          {"VolumetricRepresentation {{minX, maxX}, {minY, maxY}, {minZ, maxZ}}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "minX", MAPPING_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "maxX", MAPPING_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "minY", MAPPING_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "maxY", MAPPING_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "minZ", MAPPING_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "maxZ", MAPPING_EDGE_COLOR}}}},
                                         {{COLLECTION_ROOT, {}}},
                                         {{COLLECTION_ROOT, mappingPath + MAPPING_ROOT, MAPPING_EDGE_COLOR}}};

    CHECK (result == expected);
}

END_SUITE
