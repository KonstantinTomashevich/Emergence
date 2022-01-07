#include <cassert>

#include <RecordCollection/Visualization.hpp>

#include <Warehouse/VisualizationDriver.hpp>

namespace Emergence::Galleon
{
using namespace Container::Literals;
using namespace VisualGraph::Common::Constants;

constexpr const char *CONTAINER_ROOT_NODE = ".";

static Container::String GraphId (const SingletonContainer &_container)
{
    return "SingletonContainer {"_s + *_container.GetTypeMapping ().GetName () + "}";
}

static Container::String GraphId (const ShortTermContainer &_container)
{
    return "ShortTermContainer {"_s + *_container.GetTypeMapping ().GetName () + "}";
}

static Container::String GraphId (const LongTermContainer &_container)
{
    return "LongTermContainer {"_s + *_container.GetTypeMapping ().GetName () + "}";
}

void VisualizationDriver::PostProcess (VisualGraph::Graph &_graph, const CargoDeck &_cargoDeck) noexcept
{
    auto setupContainerGraph = [] (VisualGraph::Graph &_graph, const auto &_container)
    {
        _graph.id = GraphId (_container);
        VisualGraph::Node &root = _graph.nodes.emplace_back ();
        root.id = CONTAINER_ROOT_NODE;

        VisualGraph::Edge &mappingEdge = _graph.edges.emplace_back ();
        mappingEdge.from = root.id;
        mappingEdge.to = Container::String (DEFAULT_ROOT_GRAPH_ID) + VisualGraph::NODE_PATH_SEPARATOR +
                         MAPPING_SUBGRAPH + VisualGraph::NODE_PATH_SEPARATOR +
                         *_container.GetTypeMapping ().GetName () + VisualGraph::NODE_PATH_SEPARATOR +
                         MAPPING_ROOT_NODE;
        mappingEdge.color = MAPPING_USAGE_COLOR;
    };

    for (const SingletonContainer *container : _cargoDeck.singletonContainers)
    {
        VisualGraph::Graph &subgraph = _graph.subgraphs.emplace_back ();
        setupContainerGraph (subgraph, *container);
    }

    for (const ShortTermContainer *container : _cargoDeck.shortTermContainers)
    {
        VisualGraph::Graph &subgraph = _graph.subgraphs.emplace_back ();
        setupContainerGraph (subgraph, *container);
    }

    for (const LongTermContainer *container : _cargoDeck.longTermContainers)
    {
        VisualGraph::Graph &subgraph = _graph.subgraphs.emplace_back ();
        setupContainerGraph (subgraph, *container);
        subgraph.subgraphs.emplace_back (RecordCollection::Visualization::GraphFromCollection (container->collection));
    }
}

template <typename ContainerType>
Container::String VisualizationDriver::GetPathToContainer (const Handling::Handle<ContainerType> &_container)
{
    return Container::String (DEFAULT_ROOT_GRAPH_ID) + VisualGraph::NODE_PATH_SEPARATOR + WAREHOUSE_REGISTRY_SUBGRAPH +
           VisualGraph::NODE_PATH_SEPARATOR + *_container->deck->GetName () + VisualGraph::NODE_PATH_SEPARATOR +
           GraphId (*_container.Get ()) + VisualGraph::NODE_PATH_SEPARATOR;
}

template <typename Query>
void VisualizationDriver::LinkToContainer (VisualGraph::Graph &_graph, const Query &_query)
{
    VisualGraph::Edge &edge = _graph.edges.emplace_back ();
    edge.from = WAREHOUSE_QUERY_ROOT_NODE;
    edge.to = GetPathToContainer (_query.GetContainer ()) + CONTAINER_ROOT_NODE;
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

template <typename Query>
void VisualizationDriver::LinkToRepresentation (VisualGraph::Graph &_graph, const Query &_query)
{
    VisualGraph::Edge &edge = _graph.edges.emplace_back ();
    edge.from = WAREHOUSE_QUERY_ROOT_NODE;

    const RecordCollection::Collection &collection = _query.GetContainer ()->collection;
    edge.to = GetPathToContainer (_query.GetContainer ()) + RecordCollection::Visualization::GraphId (collection) +
              VisualGraph::NODE_PATH_SEPARATOR + RecordCollection::Visualization::GraphId (_query.representation) +
              VisualGraph::NODE_PATH_SEPARATOR + RECORD_COLLECTION_REPRESENTATION_ROOT_NODE;
}

void VisualizationDriver::PostProcess (
    VisualGraph::Graph &_graph,
    const LongTermContainer::RepresentationQueryBase<RecordCollection::LinearRepresentation> &_query)
{
    LinkToContainer (_graph, _query);
    LinkToRepresentation (_graph, _query);
}

void VisualizationDriver::PostProcess (
    VisualGraph::Graph &_graph,
    const LongTermContainer::RepresentationQueryBase<RecordCollection::PointRepresentation> &_query)
{
    LinkToContainer (_graph, _query);
    LinkToRepresentation (_graph, _query);
}

void VisualizationDriver::PostProcess (
    VisualGraph::Graph &_graph,
    const LongTermContainer::RepresentationQueryBase<RecordCollection::VolumetricRepresentation> &_query)
{
    LinkToContainer (_graph, _query);
    LinkToRepresentation (_graph, _query);
}
} // namespace Emergence::Galleon
