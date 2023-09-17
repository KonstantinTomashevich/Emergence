#include <API/Common/BlockCast.hpp>

#include <Container/StringBuilder.hpp>

#include <RecordCollection/Visualization.hpp>

namespace Emergence::RecordCollection::Visualization
{
using namespace VisualGraph::Common::Constants;

static Container::StringBuilder GetPathToMappings ()
{
    return EMERGENCE_BEGIN_BUILDING_STRING (DEFAULT_ROOT_GRAPH_ID, VisualGraph::NODE_PATH_SEPARATOR, MAPPING_SUBGRAPH,
                                            VisualGraph::NODE_PATH_SEPARATOR);
}

VisualGraph::Graph GraphFromCollection (const Collection &_collection)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_collection);

    VisualGraph::Node &root = graph.nodes.emplace_back ();
    root.id = RECORD_COLLECTION_ROOT_NODE;

    VisualGraph::Edge &mappingEdge = graph.edges.emplace_back ();
    mappingEdge.from = root.id;
    mappingEdge.to =
        GetPathToMappings ()
            .Append (_collection.GetTypeMapping ().GetName (), VisualGraph::NODE_PATH_SEPARATOR, MAPPING_ROOT_NODE)
            .Get ();

    mappingEdge.color = MAPPING_USAGE_COLOR;
    for (auto iterator = _collection.LinearRepresentationBegin (); iterator != _collection.LinearRepresentationEnd ();
         ++iterator)
    {
        graph.subgraphs.emplace_back (GraphFromLinearRepresentation (*iterator));
    }

    for (auto iterator = _collection.PointRepresentationBegin (); iterator != _collection.PointRepresentationEnd ();
         ++iterator)
    {
        graph.subgraphs.emplace_back (GraphFromPointRepresentation (*iterator));
    }

    for (auto iterator = _collection.SignalRepresentationBegin (); iterator != _collection.SignalRepresentationEnd ();
         ++iterator)
    {
        graph.subgraphs.emplace_back (GraphFromSignalRepresentation (*iterator));
    }

    for (auto iterator = _collection.VolumetricRepresentationBegin ();
         iterator != _collection.VolumetricRepresentationEnd (); ++iterator)
    {
        graph.subgraphs.emplace_back (GraphFromVolumetricRepresentation (*iterator));
    }

    return graph;
}

static VisualGraph::Edge ConnectRepresentationToField (const char *_typeName, const char *_fieldName)
{
    VisualGraph::Edge edge;
    edge.from = RECORD_COLLECTION_REPRESENTATION_ROOT_NODE;
    edge.to = GetPathToMappings ().Append (_typeName, VisualGraph::NODE_PATH_SEPARATOR, _fieldName).Get ();
    edge.color = VisualGraph::Common::Constants::MAPPING_USAGE_COLOR;
    return edge;
}

VisualGraph::Graph GraphFromLinearRepresentation (const LinearRepresentation &_representation)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_representation);
    graph.nodes.emplace_back ().id = RECORD_COLLECTION_REPRESENTATION_ROOT_NODE;

    graph.edges.emplace_back (ConnectRepresentationToField (*_representation.GetTypeMapping ().GetName (),
                                                            *_representation.GetKeyField ().GetName ()));

    return graph;
}

VisualGraph::Graph GraphFromSignalRepresentation (const SignalRepresentation &_representation)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_representation);
    graph.nodes.emplace_back ().id = RECORD_COLLECTION_REPRESENTATION_ROOT_NODE;

    graph.edges.emplace_back (ConnectRepresentationToField (*_representation.GetTypeMapping ().GetName (),
                                                            *_representation.GetKeyField ().GetName ()));

    return graph;
}

VisualGraph::Graph GraphFromPointRepresentation (const PointRepresentation &_representation)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_representation);
    graph.nodes.emplace_back ().id = RECORD_COLLECTION_REPRESENTATION_ROOT_NODE;

    for (auto iterator = _representation.KeyFieldBegin (); iterator != _representation.KeyFieldEnd (); ++iterator)
    {
        graph.edges.emplace_back (
            ConnectRepresentationToField (*_representation.GetTypeMapping ().GetName (), *(*iterator).GetName ()));
    }

    return graph;
}

VisualGraph::Graph GraphFromVolumetricRepresentation (const VolumetricRepresentation &_representation)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_representation);
    graph.nodes.emplace_back ().id = RECORD_COLLECTION_REPRESENTATION_ROOT_NODE;

    for (auto iterator = _representation.DimensionBegin (); iterator != _representation.DimensionEnd (); ++iterator)
    {
        VolumetricRepresentation::DimensionIterator::Dimension dimension = *iterator;
        graph.edges.emplace_back (ConnectRepresentationToField (*_representation.GetTypeMapping ().GetName (),
                                                                *dimension.minField.GetName ()));

        graph.edges.emplace_back (ConnectRepresentationToField (*_representation.GetTypeMapping ().GetName (),
                                                                *dimension.maxField.GetName ()));
    }

    return graph;
}

Container::String GraphId (const Collection &_collection)
{
    return EMERGENCE_BUILD_STRING ("RecordCollection {", _collection.GetTypeMapping ().GetName (), "}");
}

Container::String GraphId (const LinearRepresentation &_representation)
{
    return EMERGENCE_BUILD_STRING ("LinearRepresentation {", _representation.GetKeyField ().GetName (), "}");
}

Container::String GraphId (const PointRepresentation &_representation)
{
    Container::StringBuilder builder = EMERGENCE_BEGIN_BUILDING_STRING ("PointRepresentation {");
    bool firstKeyField = true;

    for (auto iterator = _representation.KeyFieldBegin (); iterator != _representation.KeyFieldEnd (); ++iterator)
    {
        if (!firstKeyField)
        {
            builder.Append (", ");
        }
        else
        {
            firstKeyField = false;
        }

        builder.Append ((*iterator).GetName ());
    }

    return builder.Append ("}").Get ();
}

Container::String GraphId (const SignalRepresentation &_representation)
{
    const std::array<std::uint8_t, sizeof (std::uint64_t)> &signaledValue = _representation.GetSignaledValue ();
    return EMERGENCE_BUILD_STRING (
        "SignalRepresentation {", _representation.GetKeyField ().GetName (), " = ",
        Container::StringBuilder::FieldPointer {&signaledValue, _representation.GetKeyField ()}, "}");
}

Container::String GraphId (const VolumetricRepresentation &_representation)
{
    Container::StringBuilder builder = EMERGENCE_BEGIN_BUILDING_STRING ("VolumetricRepresentation {");
    bool firstDimension = true;

    for (auto iterator = _representation.DimensionBegin (); iterator != _representation.DimensionEnd (); ++iterator)
    {
        if (!firstDimension)
        {
            builder.Append (", ");
        }
        else
        {
            firstDimension = false;
        }

        VolumetricRepresentation::DimensionIterator::Dimension dimension = *iterator;
        builder.Append ("{", dimension.minField.GetName (), ", ", dimension.maxField.GetName (), "}");
    }

    return builder.Append ("}").Get ();
}
} // namespace Emergence::RecordCollection::Visualization
