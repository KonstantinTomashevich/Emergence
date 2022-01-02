#include <Graph/DOTExporter.hpp>

#include <Log/Log.hpp>

namespace Emergence::Graph::DOTExporter
{
const Memory::UniqueString Context::ALLOCATION_GROUP {"DOTExporterContext"};

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
            Log::GlobalLogger::Log (Log::Level::ERROR, "Id \"" + _id + "\" contains forbidden symbols!");
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
        if (!IsIdValid (subgraph.id))
        {
            return false;
        }

        if (!usedIds.emplace (subgraph.id).second)
        {
            Log::GlobalLogger::Log (Log::Level::ERROR, "Subgraph id \"" + subgraph.id + "\" used more than once!");
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
            Log::GlobalLogger::Log (Log::Level::ERROR, "Node id \"" + node.id + "\" used more than once!");
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

            return relativePaths.contains (_path);
        };

        for (const VisualGraph::Edge &edge : resolvedEdges)
        {
            if (!isNodeExists (edge.from))
            {
                Log::GlobalLogger::Log (Log::Level::WARNING, "Unable to add edge \"" + edge.from + "\" -> \"" +
                                                                 edge.to + "\": source node not found!");
                continue;
            }

            if (!isNodeExists (edge.to))
            {
                Log::GlobalLogger::Log (Log::Level::WARNING, "Unable to add edge \"" + edge.from + "\" -> \"" +
                                                                 edge.to + "\": target node not found!");
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
} // namespace Emergence::Graph::DOTExporter
