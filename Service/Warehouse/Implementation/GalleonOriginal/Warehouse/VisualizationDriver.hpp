#include <Galleon/CargoDeck.hpp>

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

    static void PostProcess (
        VisualGraph::Graph &_graph,
        const LongTermContainer::RepresentationQueryBase<RecordCollection::LinearRepresentation> &_query);

    static void PostProcess (
        VisualGraph::Graph &_graph,
        const LongTermContainer::RepresentationQueryBase<RecordCollection::PointRepresentation> &_query);

    static void PostProcess (
        VisualGraph::Graph &_graph,
        const LongTermContainer::RepresentationQueryBase<RecordCollection::VolumetricRepresentation> &_query);

private:
    // Forward declare helpers, so they will gain friend access to LongTermContainer.
    template <typename Container>
    static std::string GetPathToContainer (const Handling::Handle<Container> &_container);

    template <typename Query>
    static void LinkToContainer (VisualGraph::Graph &_graph, const Query &_query);

    template <typename Query>
    static void LinkToRepresentation (VisualGraph::Graph &_graph, const Query &_query);
};
} // namespace Emergence::Galleon
