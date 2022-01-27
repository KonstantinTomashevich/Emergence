#include <Container/StringBuilder.hpp>

#include <Warehouse/Visualization.hpp>

namespace Emergence::Warehouse::Visualization
{
using namespace VisualGraph::Common::Constants;

VisualGraph::Graph GraphFromRegistry (const Registry &_registry)
{
    VisualGraph::Graph graph;
    graph.id = *_registry.GetName ();

    VisualGraph::Node &root = graph.nodes.emplace_back ();
    root.id = WAREHOUSE_QUERY_ROOT_NODE;

    _registry.AddCustomVisualization (graph);
    return graph;
}

static Container::StringBuilder GetPathToMappings ()
{
    return EMERGENCE_BEGIN_BUILDING_STRING (DEFAULT_ROOT_GRAPH_ID, VisualGraph::NODE_PATH_SEPARATOR, MAPPING_SUBGRAPH,
                                            VisualGraph::NODE_PATH_SEPARATOR);
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
    edgeToMapping.to =
        GetPathToMappings ()
            .Append (_query.GetTypeMapping ().GetName (), VisualGraph::NODE_PATH_SEPARATOR, MAPPING_ROOT_NODE)
            .Get ();

    edgeToMapping.color = MAPPING_USAGE_COLOR;

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
    edge.to =
        GetPathToMappings ().Append (_mapping.GetName (), VisualGraph::NODE_PATH_SEPARATOR, _field.GetName ()).Get ();
    edge.color = MAPPING_USAGE_COLOR;
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

static Container::StringBuilder &AppendKeyFieldSequence (Container::StringBuilder &_output,
                                                         const KeyFieldIterator &_begin,
                                                         const KeyFieldIterator &_end)
{
    KeyFieldIterator current = _begin;
    while (current != _end)
    {
        if (current != _begin)
        {
            _output.Append (", ");
        }

        _output.Append ((*current).GetName ());
        ++current;
    }

    return _output;
}

static Container::StringBuilder &AppendDimensionSequence (Container::StringBuilder &_output,
                                                          const DimensionIterator &_begin,
                                                          const DimensionIterator &_end)
{
    DimensionIterator current = _begin;
    while (current != _end)
    {
        if (current != _begin)
        {
            _output.Append (", ");
        }

        const Dimension &dimension = *current;
        _output.Append ("{", dimension.minBorderField.GetName (), ", ", dimension.maxBorderField.GetName (), "}");
        ++current;
    }

    return _output;
}

Container::String GraphId (const FetchAscendingRangeQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("FetchAscendingRangeQuery {", _query.GetTypeMapping ().GetName (), ": ",
                                   _query.GetKeyField ().GetName (), "}");
}

Container::String GraphId (const FetchDescendingRangeQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("FetchDescendingRangeQuery {", _query.GetTypeMapping ().GetName (), ": ",
                                   _query.GetKeyField ().GetName (), "}");
}

Container::String GraphId (const FetchRayIntersectionQuery &_query)
{
    Container::StringBuilder builder =
        EMERGENCE_BEGIN_BUILDING_STRING ("FetchRayIntersectionQuery {", _query.GetTypeMapping ().GetName (), ": ");
    return AppendDimensionSequence (builder, _query.DimensionBegin (), _query.DimensionEnd ()).Append ("}").Get ();
}

Container::String GraphId (const FetchSequenceQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("FetchSequenceQuery {", _query.GetTypeMapping ().GetName (), "}");
}

Container::String GraphId (const FetchShapeIntersectionQuery &_query)
{
    Container::StringBuilder builder =
        EMERGENCE_BEGIN_BUILDING_STRING ("FetchShapeIntersectionQuery {", _query.GetTypeMapping ().GetName (), ": ");
    return AppendDimensionSequence (builder, _query.DimensionBegin (), _query.DimensionEnd ()).Append ("}").Get ();
}

Container::String GraphId (const FetchSingletonQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("FetchSingletonQuery {", _query.GetTypeMapping ().GetName (), "}");
}

Container::String GraphId (const FetchValueQuery &_query)
{
    Container::StringBuilder builder =
        EMERGENCE_BEGIN_BUILDING_STRING ("FetchValueQuery {", _query.GetTypeMapping ().GetName (), ": ");
    return AppendKeyFieldSequence (builder, _query.KeyFieldBegin (), _query.KeyFieldEnd ()).Append ("}").Get ();
}

Container::String GraphId (const InsertLongTermQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("InsertLongTermQuery {", _query.GetTypeMapping ().GetName (), "}");
}

Container::String GraphId (const InsertShortTermQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("InsertShortTermQuery {", _query.GetTypeMapping ().GetName (), "}");
}

Container::String GraphId (const ModifyAscendingRangeQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("ModifyAscendingRangeQuery {", _query.GetTypeMapping ().GetName (), ": ",
                                   _query.GetKeyField ().GetName (), "}");
}

Container::String GraphId (const ModifyDescendingRangeQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("ModifyDescendingRangeQuery {", _query.GetTypeMapping ().GetName (), ": ",
                                   _query.GetKeyField ().GetName (), "}");
}

Container::String GraphId (const ModifyRayIntersectionQuery &_query)
{
    Container::StringBuilder builder =
        EMERGENCE_BEGIN_BUILDING_STRING ("ModifyRayIntersectionQuery {", _query.GetTypeMapping ().GetName (), ": ");
    return AppendDimensionSequence (builder, _query.DimensionBegin (), _query.DimensionEnd ()).Append ("}").Get ();
}

Container::String GraphId (const ModifySequenceQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("ModifySequenceQuery {", _query.GetTypeMapping ().GetName (), "}");
}

Container::String GraphId (const ModifyShapeIntersectionQuery &_query)
{
    Container::StringBuilder builder =
        EMERGENCE_BEGIN_BUILDING_STRING ("ModifyShapeIntersectionQuery {", _query.GetTypeMapping ().GetName (), ": ");
    return AppendDimensionSequence (builder, _query.DimensionBegin (), _query.DimensionEnd ()).Append ("}").Get ();
}

Container::String GraphId (const ModifySingletonQuery &_query)
{
    return EMERGENCE_BUILD_STRING ("ModifySingletonQuery {", _query.GetTypeMapping ().GetName (), "}");
}

Container::String GraphId (const ModifyValueQuery &_query)
{
    Container::StringBuilder builder =
        EMERGENCE_BEGIN_BUILDING_STRING ("ModifyValueQuery {", _query.GetTypeMapping ().GetName (), ": ");
    return AppendKeyFieldSequence (builder, _query.KeyFieldBegin (), _query.KeyFieldEnd ()).Append ("}").Get ();
}
} // namespace Emergence::Warehouse::Visualization
