#include <Container/StringBuilder.hpp>

#include <Galleon/CargoDeck.hpp>

#include <RecordCollection/Visualization.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::Galleon
{
class VisualizationDriver
{
public:
    VisualizationDriver () = delete;

    static void PostProcess (VisualGraph::Graph &_graph, const CargoDeck &_cargoDeck) noexcept;

    static void PostProcess (VisualGraph::Graph &_graph, const SingletonContainer::FetchQuery &_query);

    static void PostProcess (VisualGraph::Graph &_graph, const SingletonContainer::ModifyQuery &_query);

    static void PostProcess (VisualGraph::Graph &_graph, const ShortTermContainer::InsertQuery &_query);

    static void PostProcess (VisualGraph::Graph &_graph, const ShortTermContainer::FetchQuery &_query);

    static void PostProcess (VisualGraph::Graph &_graph, const ShortTermContainer::ModifyQuery &_query);

    static void PostProcess (VisualGraph::Graph &_graph, const LongTermContainer::InsertQuery &_query);

    template <typename Representation>
    static void PostProcess (VisualGraph::Graph &_graph,
                             const LongTermContainer::RepresentationQueryBase<Representation> &_query);

private:
    // Forward declare helpers, so they will gain friend access to required classes.
    template <typename ContainerType>
    static Container::StringBuilder GetPathToContainer (const Handling::Handle<ContainerType> &_container);

    template <typename Query>
    static void LinkToContainer (VisualGraph::Graph &_graph, const Query &_query);

    template <typename Query>
    static void LinkToRepresentation (VisualGraph::Graph &_graph, const Query &_query);
};

constexpr const char *CONTAINER_ROOT_NODE = ".";

template <typename Representation>
void VisualizationDriver::PostProcess (VisualGraph::Graph &_graph,
                                       const LongTermContainer::RepresentationQueryBase<Representation> &_query)
{
    LinkToContainer (_graph, _query);
    LinkToRepresentation (_graph, _query);
}

inline Container::String GraphId (const SingletonContainer &_container)
{
    return EMERGENCE_BUILD_STRING ("SingletonContainer {", _container.GetTypeMapping ().GetName (), "}");
}

inline Container::String GraphId (const ShortTermContainer &_container)
{
    return EMERGENCE_BUILD_STRING ("ShortTermContainer {", _container.GetTypeMapping ().GetName (), "}");
}

inline Container::String GraphId (const LongTermContainer &_container)
{
    return EMERGENCE_BUILD_STRING ("LongTermContainer {", _container.GetTypeMapping ().GetName (), "}");
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

template <typename Query>
void VisualizationDriver::LinkToContainer (VisualGraph::Graph &_graph, const Query &_query)
{
    using namespace VisualGraph::Common::Constants;
    VisualGraph::Edge &edge = _graph.edges.emplace_back ();
    edge.from = WAREHOUSE_QUERY_ROOT_NODE;
    edge.to = GetPathToContainer (_query.GetContainer ()).Append (CONTAINER_ROOT_NODE).Get ();
}

template <typename Query>
void VisualizationDriver::LinkToRepresentation (VisualGraph::Graph &_graph, const Query &_query)
{
    using namespace VisualGraph::Common::Constants;
    VisualGraph::Edge &edge = _graph.edges.emplace_back ();
    edge.from = WAREHOUSE_QUERY_ROOT_NODE;

    const RecordCollection::Collection &collection = _query.GetContainer ()->collection;
    Container::String collectionGraphId = RecordCollection::Visualization::GraphId (collection);
    Container::String representationGraphId = RecordCollection::Visualization::GraphId (_query.representation);

    edge.to = GetPathToContainer (_query.GetContainer ())
                  .Append (collectionGraphId + VisualGraph::NODE_PATH_SEPARATOR + representationGraphId +
                           VisualGraph::NODE_PATH_SEPARATOR + RECORD_COLLECTION_REPRESENTATION_ROOT_NODE)
                  .Get ();
}
} // namespace Emergence::Galleon
