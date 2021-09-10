#include <Query/Test/DataTypes.hpp>

#include <RecordCollection/Test/Scenario.hpp>
#include <RecordCollection/Test/Visualization.hpp>
#include <RecordCollection/Visualization.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence;
using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::VisualizationTestIncludeMarker () noexcept
{
    return true;
}

constexpr const char *COLLECTION_ROOT = RecordCollection::Visualization::COLLECTION_ROOT_NODE;
constexpr const char *REPRESENTATION_ROOT = RecordCollection::Visualization::REPRESENTATION_ROOT_NODE;
constexpr const char *FIELD_EDGE_COLOR = VisualGraph::Common::Constants::MAPPING_FIELD_USAGE_COLOR;

static std::string GetPathToMappings ()
{
    using namespace VisualGraph::Common::Constants;
    return std::string (DEFAULT_ROOT_GRAPH_ID) + VisualGraph::NODE_PATH_SEPARATOR + MAPPING_SUBGRAPH +
           VisualGraph::NODE_PATH_SEPARATOR;
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

    const std::string mappingPath = GetPathToMappings () +
                                    Query::Test::PlayerWithBoundingBox::Reflect ().mapping.GetName () +
                                    VisualGraph::NODE_PATH_SEPARATOR;

    const VisualGraph::Graph expected = {
        "RecordCollection {PlayerWithBoundingBox}",
        {},
        {{"LinearRepresentation {player.name}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "player.name", FIELD_EDGE_COLOR}}},
         {"PointRepresentation {player.alive}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "player.alive", FIELD_EDGE_COLOR}}},
         {"VolumetricRepresentation {{boundingBox.minX, boundingBox.maxX}, {boundingBox.minY, boundingBox.maxY}}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "boundingBox.minX", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "boundingBox.maxX", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "boundingBox.minY", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "boundingBox.maxY", FIELD_EDGE_COLOR}}}},
        {{COLLECTION_ROOT, {}}},
        {{COLLECTION_ROOT, "LinearRepresentation {player.name}/.", {}},
         {COLLECTION_ROOT, "PointRepresentation {player.alive}/.", {}},
         {COLLECTION_ROOT,
          "VolumetricRepresentation {{boundingBox.minX, boundingBox.maxX}, {boundingBox.minY, boundingBox.maxY}}/.",
          {}}}};

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

    const std::string mappingPath =
        GetPathToMappings () + Query::Test::Player::Reflect ().mapping.GetName () + VisualGraph::NODE_PATH_SEPARATOR;

    const VisualGraph::Graph expected = {
        "RecordCollection {Player}",
        {},
        {{"LinearRepresentation {id}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "id", FIELD_EDGE_COLOR}}},
         {"LinearRepresentation {name}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "name", FIELD_EDGE_COLOR}}}},
        {{COLLECTION_ROOT, {}}},
        {{COLLECTION_ROOT, "LinearRepresentation {id}/.", {}}, {COLLECTION_ROOT, "LinearRepresentation {name}/.", {}}}};

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

    const std::string mappingPath =
        GetPathToMappings () + Query::Test::Player::Reflect ().mapping.GetName () + VisualGraph::NODE_PATH_SEPARATOR;

    const VisualGraph::Graph expected = {"RecordCollection {Player}",
                                         {},
                                         {{"PointRepresentation {id, name}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "id", FIELD_EDGE_COLOR},
                                            {REPRESENTATION_ROOT, mappingPath + "name", FIELD_EDGE_COLOR}}},
                                          {"PointRepresentation {name}",
                                           {},
                                           {},
                                           {{REPRESENTATION_ROOT, {}}},
                                           {{REPRESENTATION_ROOT, mappingPath + "name", FIELD_EDGE_COLOR}}}},
                                         {{COLLECTION_ROOT, {}}},
                                         {{COLLECTION_ROOT, "PointRepresentation {id, name}/.", {}},
                                          {COLLECTION_ROOT, "PointRepresentation {name}/.", {}}}};

    CHECK (result == expected);
}

TEST_CASE (MultipleInstancesOfVolumetricRepresentation)
{
    const std::vector<Query::Test::Sources::Volumetric::Dimension> dimensions2d = {
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

    std::vector<Query::Test::Sources::Volumetric::Dimension> dimensions3d = dimensions2d;
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

    const std::string mappingPath = GetPathToMappings () + Query::Test::BoundingBox::Reflect ().mapping.GetName () +
                                    VisualGraph::NODE_PATH_SEPARATOR;

    const VisualGraph::Graph expected = {
        "RecordCollection {BoundingBox}",
        {},
        {{"VolumetricRepresentation {{minX, maxX}, {minY, maxY}}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "minX", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "maxX", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "minY", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "maxY", FIELD_EDGE_COLOR}}},
         {"VolumetricRepresentation {{minX, maxX}, {minY, maxY}, {minZ, maxZ}}",
          {},
          {},
          {{REPRESENTATION_ROOT, {}}},
          {{REPRESENTATION_ROOT, mappingPath + "minX", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "maxX", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "minY", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "maxY", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "minZ", FIELD_EDGE_COLOR},
           {REPRESENTATION_ROOT, mappingPath + "maxZ", FIELD_EDGE_COLOR}}}},
        {{COLLECTION_ROOT, {}}},
        {{COLLECTION_ROOT, "VolumetricRepresentation {{minX, maxX}, {minY, maxY}}/.", {}},
         {COLLECTION_ROOT, "VolumetricRepresentation {{minX, maxX}, {minY, maxY}, {minZ, maxZ}}/.", {}}}};

    CHECK (result == expected);
}

END_SUITE
