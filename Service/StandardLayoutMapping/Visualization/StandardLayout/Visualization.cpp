#include <cstdlib>
#include <functional>

#include <StandardLayout/Visualization.hpp>

namespace Emergence::StandardLayout::Visualization
{
VisualGraph::Graph GraphFromMapping (const Mapping &_mapping)
{
    using namespace VisualGraph::Common::Constants;

    VisualGraph::Graph graph;
    graph.id = *_mapping.GetName ();
    graph.nodes.emplace_back (VisualGraph::Node {MAPPING_ROOT_NODE, {}});

    for (StandardLayout::Field field : _mapping)
    {
        VisualGraph::Node &node = graph.nodes.emplace_back ();
        node.id = *field.GetName ();

        const char *lastSeparator = strrchr (*field.GetName (), PROJECTION_NAME_SEPARATOR);
        VisualGraph::Edge &edge = graph.edges.emplace_back ();
        edge.to = node.id;

        if (lastSeparator)
        {
            node.label = lastSeparator + 1u;
            edge.from = {*field.GetName (), static_cast<std::size_t> (lastSeparator - *field.GetName ())};
        }
        else
        {
            edge.from = MAPPING_ROOT_NODE;
        }
    }

    return graph;
}
} // namespace Emergence::StandardLayout::Visualization
