#include <Export/Graph.hpp>

#include <Graph/DOTExporter.hpp>

namespace Emergence::Export::Graph
{
bool Export (const VisualGraph::Graph &_graph, std::ostream &_output) noexcept
{
    return Emergence::Graph::DOTExporter::Context::Execute (_graph, _output);
}
} // namespace Emergence::Export::Graph
