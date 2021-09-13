#include <Warehouse/Visualization.hpp>

namespace Emergence::Warehouse::Visualization
{
using namespace VisualGraph::Common::Constants;

VisualGraph::Graph GraphFromRegistry (const Registry &_registry)
{
    VisualGraph::Graph graph;
    graph.id = _registry.GetName ();

    VisualGraph::Node &root = graph.nodes.emplace_back ();
    root.id = WAREHOUSE_QUERY_ROOT_NODE;

    _registry.AddCustomVisualization (graph);
    return graph;
}

static std::string GetPathToMappings ()
{
    return std::string (DEFAULT_ROOT_GRAPH_ID) + VisualGraph::NODE_PATH_SEPARATOR + MAPPING_SUBGRAPH +
           VisualGraph::NODE_PATH_SEPARATOR;
}

template <typename Query>
VisualGraph::Graph BaseGraphForQuery (const Query &_query)
{
    VisualGraph::Graph graph;
    graph.id = GraphId (_query);

    VisualGraph::Node &root = graph.nodes.emplace_back ();
    root.id = WAREHOUSE_QUERY_ROOT_NODE;

    VisualGraph::Edge &edgeToMapping = graph.edges.emplace_back ();
    edgeToMapping.from = root.id;
    edgeToMapping.to = GetPathToMappings () + _query.GetTypeMapping ().GetName () + VisualGraph::NODE_PATH_SEPARATOR +
                       MAPPING_ROOT_NODE;
    edgeToMapping.color = VisualGraph::Common::Constants::MAPPING_USAGE_COLOR;

    // Storages are not exposed by API, therefore these connections can only be added by implementation customization.
    _query.AddCustomVisualization (graph);
    return graph;
}

static void ConnectToField (VisualGraph::Graph &_graph,
                            const StandardLayout::Mapping &_mapping,
                            const StandardLayout::Field &_field)
{
    VisualGraph::Edge &edge = _graph.edges.emplace_back ();
    edge.from = WAREHOUSE_QUERY_ROOT_NODE;
    edge.to = GetPathToMappings () + _mapping.GetName () + VisualGraph::NODE_PATH_SEPARATOR + _field.GetName ();
    edge.color = VisualGraph::Common::Constants::MAPPING_USAGE_COLOR;
}

static VisualGraph::Graph ConnectToFields (VisualGraph::Graph _graph,
                                           const StandardLayout::Mapping &_mapping,
                                           const KeyFieldIterator &_begin,
                                           const KeyFieldIterator &_end)
{
    KeyFieldIterator current = _begin;
    while (current != _end)
    {
        ConnectToField (_graph, _mapping, *current);
        ++current;
    }

    return _graph;
}

static VisualGraph::Graph ConnectToFields (VisualGraph::Graph _graph,
                                           const StandardLayout::Mapping &_mapping,
                                           const DimensionIterator &_begin,
                                           const DimensionIterator &_end)
{
    DimensionIterator current = _begin;
    while (current != _end)
    {
        const Dimension &dimension = *current;
        ConnectToField (_graph, _mapping, dimension.minBorderField);
        ConnectToField (_graph, _mapping, dimension.maxBorderField);
        ++current;
    }

    return _graph;
}

VisualGraph::Graph GraphFromQuery (const FetchAscendingRangeQuery &_query)
{
    VisualGraph::Graph graph = BaseGraphForQuery (_query);
    ConnectToField (graph, _query.GetTypeMapping (), _query.GetKeyField ());
    return graph;
}

VisualGraph::Graph GraphFromQuery (const FetchDescendingRangeQuery &_query)
{
    VisualGraph::Graph graph = BaseGraphForQuery (_query);
    ConnectToField (graph, _query.GetTypeMapping (), _query.GetKeyField ());
    return graph;
}

VisualGraph::Graph GraphFromQuery (const FetchRayIntersectionQuery &_query)
{
    return ConnectToFields (BaseGraphForQuery (_query), _query.GetTypeMapping (), _query.DimensionBegin (),
                            _query.DimensionEnd ());
}

VisualGraph::Graph GraphFromQuery (const FetchSequenceQuery &_query)
{
    return BaseGraphForQuery (_query);
}

VisualGraph::Graph GraphFromQuery (const FetchShapeIntersectionQuery &_query)
{
    return ConnectToFields (BaseGraphForQuery (_query), _query.GetTypeMapping (), _query.DimensionBegin (),
                            _query.DimensionEnd ());
}

VisualGraph::Graph GraphFromQuery (const FetchSingletonQuery &_query)
{
    return BaseGraphForQuery (_query);
}

VisualGraph::Graph GraphFromQuery (const FetchValueQuery &_query)
{
    return ConnectToFields (BaseGraphForQuery (_query), _query.GetTypeMapping (), _query.KeyFieldBegin (),
                            _query.KeyFieldEnd ());
}

VisualGraph::Graph GraphFromQuery (const InsertLongTermQuery &_query)
{
    return BaseGraphForQuery (_query);
}

VisualGraph::Graph GraphFromQuery (const InsertShortTermQuery &_query)
{
    return BaseGraphForQuery (_query);
}

VisualGraph::Graph GraphFromQuery (const ModifyAscendingRangeQuery &_query)
{
    VisualGraph::Graph graph = BaseGraphForQuery (_query);
    ConnectToField (graph, _query.GetTypeMapping (), _query.GetKeyField ());
    return graph;
}

VisualGraph::Graph GraphFromQuery (const ModifyDescendingRangeQuery &_query)
{
    VisualGraph::Graph graph = BaseGraphForQuery (_query);
    ConnectToField (graph, _query.GetTypeMapping (), _query.GetKeyField ());
    return graph;
}

VisualGraph::Graph GraphFromQuery (const ModifyRayIntersectionQuery &_query)
{
    return ConnectToFields (BaseGraphForQuery (_query), _query.GetTypeMapping (), _query.DimensionBegin (),
                            _query.DimensionEnd ());
}

VisualGraph::Graph GraphFromQuery (const ModifySequenceQuery &_query)
{
    return BaseGraphForQuery (_query);
}

VisualGraph::Graph GraphFromQuery (const ModifyShapeIntersectionQuery &_query)
{
    return ConnectToFields (BaseGraphForQuery (_query), _query.GetTypeMapping (), _query.DimensionBegin (),
                            _query.DimensionEnd ());
}

VisualGraph::Graph GraphFromQuery (const ModifySingletonQuery &_query)
{
    return BaseGraphForQuery (_query);
}

VisualGraph::Graph GraphFromQuery (const ModifyValueQuery &_query)
{
    return ConnectToFields (BaseGraphForQuery (_query), _query.GetTypeMapping (), _query.KeyFieldBegin (),
                            _query.KeyFieldEnd ());
}

static void AppendKeyFieldSequence (std::string &_output, const KeyFieldIterator &_begin, const KeyFieldIterator &_end)
{
    KeyFieldIterator current = _begin;
    while (current != _end)
    {
        if (current != _begin)
        {
            _output += ", ";
        }

        _output += (*current).GetName ();
        ++current;
    }
}

static void AppendDimensionSequence (std::string &_output,
                                     const DimensionIterator &_begin,
                                     const DimensionIterator &_end)
{
    DimensionIterator current = _begin;
    while (current != _end)
    {
        if (current != _begin)
        {
            _output += ", ";
        }

        const Dimension &dimension = *current;
        _output +=
            std::string ("{") + dimension.minBorderField.GetName () + ", " + dimension.maxBorderField.GetName () + "}";
        ++current;
    }
}

std::string GraphId (const FetchAscendingRangeQuery &_query)
{
    return std::string ("FetchAscendingRangeQuery {") + _query.GetTypeMapping ().GetName () + ": " +
           _query.GetKeyField ().GetName () + "}";
}

std::string GraphId (const FetchDescendingRangeQuery &_query)
{
    return std::string ("FetchDescendingRangeQuery {") + _query.GetTypeMapping ().GetName () + ": " +
           _query.GetKeyField ().GetName () + "}";
}

std::string GraphId (const FetchRayIntersectionQuery &_query)
{
    std::string result = std::string ("FetchRayIntersectionQuery {") + _query.GetTypeMapping ().GetName () + ": ";
    AppendDimensionSequence (result, _query.DimensionBegin (), _query.DimensionEnd ());
    return result + "}";
}

std::string GraphId (const FetchSequenceQuery &_query)
{
    return std::string ("FetchSequenceQuery {") + _query.GetTypeMapping ().GetName () + "}";
}

std::string GraphId (const FetchShapeIntersectionQuery &_query)
{
    std::string result = std::string ("FetchShapeIntersectionQuery {") + _query.GetTypeMapping ().GetName () + ": ";
    AppendDimensionSequence (result, _query.DimensionBegin (), _query.DimensionEnd ());
    return result + "}";
}

std::string GraphId (const FetchSingletonQuery &_query)
{
    return std::string ("FetchSingletonQuery {") + _query.GetTypeMapping ().GetName () + "}";
}

std::string GraphId (const FetchValueQuery &_query)
{
    std::string result = std::string ("FetchValueQuery {") + _query.GetTypeMapping ().GetName () + ": ";
    AppendKeyFieldSequence (result, _query.KeyFieldBegin (), _query.KeyFieldEnd ());
    return result + "}";
}

std::string GraphId (const InsertLongTermQuery &_query)
{
    return std::string ("InsertLongTermQuery {") + _query.GetTypeMapping ().GetName () + "}";
}

std::string GraphId (const InsertShortTermQuery &_query)
{
    return std::string ("InsertShortTermQuery {") + _query.GetTypeMapping ().GetName () + "}";
}

std::string GraphId (const ModifyAscendingRangeQuery &_query)
{
    return std::string ("ModifyAscendingRangeQuery {") + _query.GetTypeMapping ().GetName () + ": " +
           _query.GetKeyField ().GetName () + "}";
}

std::string GraphId (const ModifyDescendingRangeQuery &_query)
{
    return std::string ("ModifyDescendingRangeQuery {") + _query.GetTypeMapping ().GetName () + ": " +
           _query.GetKeyField ().GetName () + "}";
}

std::string GraphId (const ModifyRayIntersectionQuery &_query)
{
    std::string result = std::string ("ModifyRayIntersectionQuery {") + _query.GetTypeMapping ().GetName () + ": ";
    AppendDimensionSequence (result, _query.DimensionBegin (), _query.DimensionEnd ());
    return result + "}";
}

std::string GraphId (const ModifySequenceQuery &_query)
{
    return std::string ("ModifySequenceQuery {") + _query.GetTypeMapping ().GetName () + "}";
}

std::string GraphId (const ModifyShapeIntersectionQuery &_query)
{
    std::string result = std::string ("ModifyShapeIntersectionQuery {") + _query.GetTypeMapping ().GetName () + ": ";
    AppendDimensionSequence (result, _query.DimensionBegin (), _query.DimensionEnd ());
    return result + "}";
}

std::string GraphId (const ModifySingletonQuery &_query)
{
    return std::string ("ModifySingletonQuery {") + _query.GetTypeMapping ().GetName () + "}";
}

std::string GraphId (const ModifyValueQuery &_query)
{
    std::string result = std::string ("ModifyValueQuery {") + _query.GetTypeMapping ().GetName () + ": ";
    AppendKeyFieldSequence (result, _query.KeyFieldBegin (), _query.KeyFieldEnd ());
    return result + "}";
}
} // namespace Emergence::Warehouse::Visualization
