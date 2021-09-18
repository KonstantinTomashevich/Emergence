#include <unordered_set>

#include <Export/Graph.hpp>

namespace Emergence::Export::Graph
{
class Context
{
public:
    /// \brief Tries to export given graph to given stream.
    static bool Execute (const VisualGraph::Graph &_graph, std::ostream &_output) noexcept;

private:
    /// \return Is given id correct?
    static bool IsIdValid (const std::string &_id) noexcept;

    /// \return Are local ids for subgraphs and nodes of given graph correct and unique?
    static bool CheckIds (const VisualGraph::Graph &_graph) noexcept;

    explicit Context (std::ostream &_output);

    /// \brief Exports given graph and its subgraphs to ::output.
    /// \return If export was done successfully, returns set of all existing node relative paths for this graph.
    std::optional<std::unordered_set<std::string>> Process (const VisualGraph::Graph &_path,
                                                            std::string _pathPrefix = "",
                                                            const std::string &_outerIndentation = "");

    std::ostream &output;

    /// \details DOT creates nodes on demand, therefore referencing node by the absolute path before it was declared in
    ///          its subgraph will result in node creation in incorrect subgraph. We use the simplest solution for this
    ///          problem: all edges (doesn't matter in which graph edge was declared) are defined in root graph after
    ///          all subgraphs and nodes. Therefore we store all resolved edges in this array until all subgraphs and
    ///          nodes are defined.
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
        if (symbol == '\"' || symbol == VisualGraph::NODE_PATH_SEPARATOR)
        {
            // TODO: Log error.
            return false;
        }
    }

    return true;
}

bool Context::CheckIds (const VisualGraph::Graph &_graph) noexcept
{
    std::unordered_set<std::string> usedIds;
    for (const VisualGraph::Graph &subgraph : _graph.subgraphs)
    {
        if (!IsIdValid (subgraph.id) || !usedIds.emplace (subgraph.id).second)
        {
            // TODO: Log error.
            return false;
        }
    }

    for (const VisualGraph::Node &node : _graph.nodes)
    {
        if (!IsIdValid (node.id) || !usedIds.emplace (node.id).second)
        {
            // TODO: Log error.
            return false;
        }
    }

    return true;
}

Context::Context (std::ostream &_output) : output (_output)
{
}

std::optional<std::unordered_set<std::string>> Context::Process (const VisualGraph::Graph &_graph,
                                                                 std::string _pathPrefix,
                                                                 const std::string &_outerIndentation)
{
    if (!CheckIds (_graph))
    {
        return std::nullopt;
    }

    const bool isSubgraph = !_pathPrefix.empty ();
    std::string indentation = _outerIndentation + "    ";

    if (isSubgraph)
    {
        output << _outerIndentation << "subgraph "
               << "\"cluster_" << _pathPrefix << _graph.id << "\" {" << std::endl;
    }
    else
    {
        output << _outerIndentation << "digraph "
               << "\"" << _graph.id << "\" {" << std::endl;
    }

    output << indentation << "label=\"" << _graph.label.value_or (_graph.id) << "\";" << std::endl;
    std::unordered_set<std::string> relativePaths;
    _pathPrefix += _graph.id + VisualGraph::NODE_PATH_SEPARATOR;

    for (const VisualGraph::Graph &subgraph : _graph.subgraphs)
    {
        std::optional<std::unordered_set<std::string>> subgraphRelativePaths =
            Process (subgraph, _pathPrefix, indentation);

        if (!subgraphRelativePaths)
        {
            return std::nullopt;
        }

        for (const std::string &subgraphRelativePath : subgraphRelativePaths.value ())
        {
            relativePaths.emplace (subgraph.id + VisualGraph::NODE_PATH_SEPARATOR + subgraphRelativePath);
        }
    }

    for (const VisualGraph::Node &node : _graph.nodes)
    {
        output << indentation << "\"" << _pathPrefix + node.id << "\" [";
        output << "label=\"" << node.label.value_or (node.id) << "\" ";

        output << "];" << std::endl;
        relativePaths.emplace (node.id);
    }

    for (VisualGraph::Edge edge : _graph.edges)
    {
        auto patchPath = [&relativePaths, &_pathPrefix] (const std::string &_path) -> std::string
        {
            return relativePaths.contains (_path) ? _pathPrefix + _path : _path;
        };

        edge.from = patchPath (edge.from);
        edge.to = patchPath (edge.to);
        resolvedEdges.emplace_back (std::move (edge));
    }

    if (!isSubgraph)
    {
        const std::string absolutePrefix = _graph.id + VisualGraph::NODE_PATH_SEPARATOR;
        auto isNodeExists = [&absolutePrefix, &relativePaths] (const std::string &_path)
        {
            if (_path.starts_with (absolutePrefix))
            {
                return relativePaths.contains (_path.substr (absolutePrefix.size ()));
            }
            else
            {
                return relativePaths.contains (_path);
            }
        };

        for (const VisualGraph::Edge &edge : resolvedEdges)
        {
            if (!isNodeExists (edge.from))
            {
                // TODO: Log warning.
                continue;
            }

            if (!isNodeExists (edge.to))
            {
                // TODO: Log warning.
                continue;
            }

            output << indentation << "\"" << edge.from << "\" -> \"" << edge.to << "\" [";
            if (edge.color)
            {
                output << "color=\"" << edge.color.value () << "\" ";
            }

            output << "];" << std::endl;
        }
    }

    output << _outerIndentation << "}" << std::endl;
    return std::move (relativePaths);
}

bool Export (const VisualGraph::Graph &_graph, std::ostream &_output) noexcept
{
    return Context::Execute (_graph, _output);
}
} // namespace Emergence::Export::Graph
