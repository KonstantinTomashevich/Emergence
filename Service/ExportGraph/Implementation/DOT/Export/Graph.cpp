#include <unordered_set>

#include <Export/Graph.hpp>

namespace Emergence::Export::Graph
{
// TODO: Add error logging (no logging service yet).

static bool IsIdValid (const std::string &_id)
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

static bool CheckIds (const VisualGraph::Graph &_graph)
{
    std::unordered_set<std::string> usedIds;
    for (const VisualGraph::Graph &_subgraph : _graph.subgraphs)
    {
        if (!IsIdValid (_subgraph.id))
        {
            return false;
        }

        if (!usedIds.emplace (_subgraph.id).second)
        {
            return false;
        }
    }

    for (const VisualGraph::Node &_node : _graph.nodes)
    {
        if (!IsIdValid (_node.id))
        {
            return false;
        }

        if (!usedIds.emplace (_node.id).second)
        {
            return false;
        }
    }

    return true;
}

static std::optional<std::unordered_set<std::string>> ExportGraph (const VisualGraph::Graph &_graph,
                                                                   std::ostream &_output,
                                                                   std::string pathPrefix,
                                                                   const std::string &outerIndentation)
{
    if (!CheckIds (_graph))
    {
        return std::nullopt;
    }

    if (pathPrefix.empty ())
    {
        _output << outerIndentation << "digraph ";
    }
    else
    {
        _output << outerIndentation << "subgraph ";
    }

    std::string indentation = outerIndentation + "    ";
    _output << "\"" << _graph.id << "\" {" << std::endl << indentation << "compound=true;" << std::endl;

    if (_graph.label)
    {
        _output << indentation << "label=\"" << _graph.label.value () << "\";" << std::endl;
    }

    std::unordered_set<std::string> nodesInLocalContext;
    pathPrefix += _graph.id + '/';

    for (const VisualGraph::Graph &_subgraph : _graph.subgraphs)
    {
        std::optional<std::unordered_set<std::string>> subgraphNodes =
            ExportGraph (_subgraph, _output, pathPrefix, indentation);

        if (!subgraphNodes)
        {
            return std::nullopt;
        }

        for (const std::string &subgraphNode : subgraphNodes.value ())
        {
            nodesInLocalContext.emplace (_graph.id + '/' + subgraphNode);
        }
    }

    for (const VisualGraph::Node &_node : _graph.nodes)
    {
        _output << indentation << "\"" << pathPrefix + _node.id << "\" [";
        if (_node.label)
        {
            _output << "label=\"" << _node.label.value () << "\" ";
        }

        _output << "];" << std::endl;
        nodesInLocalContext.emplace (_node.id);
    }

    for (const VisualGraph::Edge &_edge : _graph.edges)
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

        _output << indentation << "\"" << PatchContext (_edge.from) << "\" -> \"" << PatchContext (_edge.to) << "\";"
                << std::endl;
    }

    _output << outerIndentation << "}";
    return std::move (nodesInLocalContext);
}

bool Export (const VisualGraph::Graph &_graph, std::ostream &_output) noexcept
{
    return ExportGraph (_graph, _output, "", "") != std::nullopt;
}
} // namespace Emergence::Export::Graph
