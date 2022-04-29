#include <Warehouse/VisualizationDriver.hpp>

namespace Emergence::Galleon
{

static Container::String GraphId (const SingletonContainer &_container)
{
    return EMERGENCE_BUILD_STRING ("SingletonContainer {", _container.GetTypeMapping ().GetName (), "}");
}

static Container::String GraphId (const ShortTermContainer &_container)
{
    return EMERGENCE_BUILD_STRING ("ShortTermContainer {", _container.GetTypeMapping ().GetName (), "}");
}

static Container::String GraphId (const LongTermContainer &_container)
{
    return EMERGENCE_BUILD_STRING ("LongTermContainer {", _container.GetTypeMapping ().GetName (), "}");
}

void VisualizationDriver::PostProcess (VisualGraph::Graph &_graph, const CargoDeck &_cargoDeck) noexcept
{
    auto setupContainerGraph = [] (VisualGraph::Graph &_graph, const auto &_container)
    {
        using namespace VisualGraph::Common::Constants;
        _graph.id = GraphId (_container);
        VisualGraph::Node &root = _graph.nodes.emplace_back ();
        root.id = CONTAINER_ROOT_NODE;

        VisualGraph::Edge &mappingEdge = _graph.edges.emplace_back ();
        mappingEdge.from = root.id;

        mappingEdge.to = EMERGENCE_BUILD_STRING (
            DEFAULT_ROOT_GRAPH_ID, VisualGraph::NODE_PATH_SEPARATOR, MAPPING_SUBGRAPH, VisualGraph::NODE_PATH_SEPARATOR,
            _container.GetTypeMapping ().GetName (), VisualGraph::NODE_PATH_SEPARATOR, MAPPING_ROOT_NODE);

        mappingEdge.color = MAPPING_USAGE_COLOR;
    };

    for (const SingletonContainer &container : _cargoDeck.singleton)
    {
        VisualGraph::Graph &subgraph = _graph.subgraphs.emplace_back ();
        setupContainerGraph (subgraph, container);
    }

    for (const ShortTermContainer &container : _cargoDeck.shortTerm)
    {
        VisualGraph::Graph &subgraph = _graph.subgraphs.emplace_back ();
        setupContainerGraph (subgraph, container);
    }

    for (const LongTermContainer &container : _cargoDeck.longTerm)
    {
        VisualGraph::Graph &subgraph = _graph.subgraphs.emplace_back ();
        setupContainerGraph (subgraph, container);
        subgraph.subgraphs.emplace_back (RecordCollection::Visualization::GraphFromCollection (container.collection));
    }
}

template <typename ContainerType>
Container::StringBuilder VisualizationDriver::GetPathToContainer (const Handling::Handle<ContainerType> &_container)
{
    using namespace VisualGraph::Common::Constants;
    return EMERGENCE_BEGIN_BUILDING_STRING (DEFAULT_ROOT_GRAPH_ID, VisualGraph::NODE_PATH_SEPARATOR,
                                            WAREHOUSE_REGISTRY_SUBGRAPH, VisualGraph::NODE_PATH_SEPARATOR,
                                            _container->deck->GetName (), VisualGraph::NODE_PATH_SEPARATOR,
                                            GraphId (*_container.Get ()), VisualGraph::NODE_PATH_SEPARATOR);
}

void VisualizationDriver::PostProcess (VisualGraph::Graph &_graph, const SingletonContainer::FetchQuery &_query)
{
    LinkToContainer (_graph, _query);
}

void VisualizationDriver::PostProcess (VisualGraph::Graph &_graph, const SingletonContainer::ModifyQuery &_query)
{
    LinkToContainer (_graph, _query);
}

void VisualizationDriver::PostProcess (VisualGraph::Graph &_graph, const ShortTermContainer::InsertQuery &_query)
{
    LinkToContainer (_graph, _query);
}

void VisualizationDriver::PostProcess (VisualGraph::Graph &_graph, const ShortTermContainer::FetchQuery &_query)
{
    LinkToContainer (_graph, _query);
}

void VisualizationDriver::PostProcess (VisualGraph::Graph &_graph, const ShortTermContainer::ModifyQuery &_query)
{
    LinkToContainer (_graph, _query);
}

void VisualizationDriver::PostProcess (VisualGraph::Graph &_graph, const LongTermContainer::InsertQuery &_query)
{
    LinkToContainer (_graph, _query);
}
} // namespace Emergence::Galleon
