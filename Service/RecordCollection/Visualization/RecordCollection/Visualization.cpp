#include <RecordCollection/Visualization.hpp>

namespace Emergence::RecordCollection::Visualization
{
using namespace VisualGraph::Common::Constants;

VisualGraph::Graph GraphFromCollection (const RecordCollection::Collection &_collection)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_collection);

    VisualGraph::Node &root = graph.nodes.emplace_back ();
    root.id = COLLECTION_ROOT_NODE;

    auto connectToCollection = [&graph, &root] (const VisualGraph::Graph &_subgraph)
    {
        VisualGraph::Edge &connection = graph.edges.emplace_back ();
        connection.from = root.id;
        connection.to = _subgraph.id + VisualGraph::NODE_PATH_SEPARATOR + REPRESENTATION_ROOT_NODE;
    };

    for (auto iterator = _collection.LinearRepresentationBegin (); iterator != _collection.LinearRepresentationEnd ();
         ++iterator)
    {
        connectToCollection (graph.subgraphs.emplace_back (GraphFromLinearRepresentation (*iterator)));
    }

    for (auto iterator = _collection.PointRepresentationBegin (); iterator != _collection.PointRepresentationEnd ();
         ++iterator)
    {
        connectToCollection (graph.subgraphs.emplace_back (GraphFromPointRepresentation (*iterator)));
    }

    for (auto iterator = _collection.VolumetricRepresentationBegin ();
         iterator != _collection.VolumetricRepresentationEnd (); ++iterator)
    {
        connectToCollection (graph.subgraphs.emplace_back (GraphFromVolumetricRepresentation (*iterator)));
    }

    return graph;
}

static std::string GetPathToMappings ()
{
    return std::string (DEFAULT_ROOT_GRAPH_ID) + VisualGraph::NODE_PATH_SEPARATOR + MAPPING_SUBGRAPH +
           VisualGraph::NODE_PATH_SEPARATOR;
}

static VisualGraph::Edge ConnectRepresentationToField (const std::string &_typeName, const std::string &_fieldName)
{
    VisualGraph::Edge edge;
    edge.from = REPRESENTATION_ROOT_NODE;
    edge.to = GetPathToMappings () + _typeName + VisualGraph::NODE_PATH_SEPARATOR + _fieldName;
    edge.color = VisualGraph::Common::Constants::MAPPING_FIELD_USAGE_COLOR;
    return edge;
}

VisualGraph::Graph GraphFromLinearRepresentation (const RecordCollection::LinearRepresentation &_representation)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_representation);
    graph.nodes.emplace_back ().id = REPRESENTATION_ROOT_NODE;

    graph.edges.emplace_back (ConnectRepresentationToField (_representation.GetTypeMapping ().GetName (),
                                                            _representation.GetKeyField ().GetName ()));

    return graph;
}

VisualGraph::Graph GraphFromPointRepresentation (const RecordCollection::PointRepresentation &_representation)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_representation);
    graph.nodes.emplace_back ().id = REPRESENTATION_ROOT_NODE;

    for (auto iterator = _representation.KeyFieldBegin (); iterator != _representation.KeyFieldEnd (); ++iterator)
    {
        graph.edges.emplace_back (
            ConnectRepresentationToField (_representation.GetTypeMapping ().GetName (), (*iterator).GetName ()));
    }

    return graph;
}

VisualGraph::Graph GraphFromVolumetricRepresentation (const RecordCollection::VolumetricRepresentation &_representation)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_representation);
    graph.nodes.emplace_back ().id = REPRESENTATION_ROOT_NODE;

    for (auto iterator = _representation.DimensionBegin (); iterator != _representation.DimensionEnd (); ++iterator)
    {
        VolumetricRepresentation::DimensionIterator::Dimension dimension = *iterator;
        graph.edges.emplace_back (
            ConnectRepresentationToField (_representation.GetTypeMapping ().GetName (), dimension.minField.GetName ()));

        graph.edges.emplace_back (
            ConnectRepresentationToField (_representation.GetTypeMapping ().GetName (), dimension.maxField.GetName ()));
    }

    return graph;
}

std::string GraphId (const Collection &_collection)
{
    return std::string ("RecordCollection {") + _collection.GetTypeMapping ().GetName () + "}";
}

std::string GraphId (const LinearRepresentation &_representation)
{
    return std::string ("LinearRepresentation {") + _representation.GetKeyField ().GetName () + "}";
}

std::string GraphId (const PointRepresentation &_representation)
{
    std::string id = "PointRepresentation {";
    bool firstKeyField = true;

    for (auto iterator = _representation.KeyFieldBegin (); iterator != _representation.KeyFieldEnd (); ++iterator)
    {
        if (!firstKeyField)
        {
            id += ", ";
        }
        else
        {
            firstKeyField = false;
        }

        id += (*iterator).GetName ();
    }

    return id + "}";
}

std::string GraphId (const VolumetricRepresentation &_representation)
{
    std::string id = "VolumetricRepresentation {";
    bool firstDimension = true;

    for (auto iterator = _representation.DimensionBegin (); iterator != _representation.DimensionEnd (); ++iterator)
    {
        if (!firstDimension)
        {
            id += ", ";
        }
        else
        {
            firstDimension = false;
        }

        VolumetricRepresentation::DimensionIterator::Dimension dimension = *iterator;
        id += std::string ("{") + dimension.minField.GetName () + ", " + dimension.maxField.GetName () + "}";
    }

    return id + "}";
}
} // namespace Emergence::RecordCollection::Visualization
