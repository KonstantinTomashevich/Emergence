#include <unordered_set>

#include <Export/Graph.hpp>

namespace Emergence::Export::Graph
{
// TODO: Add error logging (no logging service yet).

class Context
{
public:
    static bool Execute (const VisualGraph::Graph &_graph, std::ostream &_output) noexcept;

private:
    static bool IsIdValid (const std::string &_id) noexcept;

    static bool CheckIds (const VisualGraph::Graph &_graph) noexcept;

    explicit Context (std::ostream &output);

    std::optional<std::unordered_set<std::string>> Process (const VisualGraph::Graph &_graph,
                                                            std::string pathPrefix = "",
                                                            const std::string &outerIndentation = "");

    std::ostream &output;
    std::vector<VisualGraph::Edge> resolvedEdges;
};

bool Context::Execute (const VisualGraph::Graph &_graph, std::ostream &_output) noexcept
{
    return Context {_output}.Process (_graph).has_value ();
}

bool Context::IsIdValid (const std::string &_id) noexcept
{
    for (const char &symbol : _id)
    {
        switch (symbol)
        {
        // Escaped quotes are technically okay, but it's easier to just forbid
        // them here and there is no real need to put quotes in ids.
        case '\"':
        // Slashes are used to define global paths.
        case '/':
            return false;
        default:
            continue;
        }
    }

    return true;
}

bool Context::CheckIds (const VisualGraph::Graph &_graph) noexcept
{
    std::unordered_set<std::string> usedIds;
    for (const VisualGraph::Graph &subgraph : _graph.subgraphs)
    {
        if (!IsIdValid (subgraph.id))
        {
            return false;
        }

        if (!usedIds.emplace (subgraph.id).second)
        {
            return false;
        }
    }

    for (const VisualGraph::Node &node : _graph.nodes)
    {
        if (!IsIdValid (node.id))
        {
            return false;
        }

        if (!usedIds.emplace (node.id).second)
        {
            return false;
        }
    }

    return true;
}

Context::Context (std::ostream &output) : output (output)
{
}

std::optional<std::unordered_set<std::string>> Context::Process (const VisualGraph::Graph &_graph,
                                                                 std::string pathPrefix,
                                                                 const std::string &outerIndentation)
{
    if (!CheckIds (_graph))
    {
        return std::nullopt;
    }

    const bool topLevel = pathPrefix.empty ();
    std::string indentation = outerIndentation + "    ";

    if (topLevel)
    {
        output << outerIndentation << "digraph "
               << "\"" << _graph.id << "\" {" << std::endl;
    }
    else
    {
        output << outerIndentation << "subgraph "
               << "\"cluster_" << _graph.id << "\" {" << std::endl;
    }

    if (_graph.label)
    {
        output << indentation << "label=\"" << _graph.label.value () << "\";" << std::endl;
    }

    std::unordered_set<std::string> nodesInLocalContext;
    pathPrefix += _graph.id + '/';

    for (const VisualGraph::Graph &subgraph : _graph.subgraphs)
    {
        std::optional<std::unordered_set<std::string>> subgraphNodes = Process (subgraph, pathPrefix, indentation);
        if (!subgraphNodes)
        {
            return std::nullopt;
        }

        for (const std::string &subgraphNode : subgraphNodes.value ())
        {
            nodesInLocalContext.emplace (subgraph.id + '/' + subgraphNode);
        }
    }

    for (const VisualGraph::Node &node : _graph.nodes)
    {
        output << indentation << "\"" << pathPrefix + node.id << "\" [";
        if (node.label)
        {
            output << "label=\"" << node.label.value () << "\" ";
        }

        output << "];" << std::endl;
        nodesInLocalContext.emplace (node.id);
    }

    for (VisualGraph::Edge edge : _graph.edges)
    {
        auto PatchContext = [&nodesInLocalContext, &pathPrefix] (const std::string &_id) -> std::string
        {
            if (nodesInLocalContext.contains (_id))
            {
                return pathPrefix + _id;
            }
            else
            {
                return _id;
            }
        };

        edge.from = PatchContext (edge.from);
        edge.to = PatchContext (edge.to);
        resolvedEdges.emplace_back (edge);
    }

    if (topLevel)
    {
        for (const VisualGraph::Edge &edge : resolvedEdges)
        {
            output << indentation << "\"" << edge.from << "\" -> \"" << edge.to << "\";" << std::endl;
        }
    }

    output << outerIndentation << "}" << std::endl;
    return std::move (nodesInLocalContext);
}

bool Export (const VisualGraph::Graph &_graph, std::ostream &_output) noexcept
{
    return Context::Execute (_graph, _output);
}
} // namespace Emergence::Export::Graph
