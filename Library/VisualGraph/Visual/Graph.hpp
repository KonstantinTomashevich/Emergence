#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Emergence::VisualGraph
{
/// \brief Defines node of a Graph.
struct Node
{
    /// \brief Node local id in its graph.
    /// \invariant Unique among nodes of parent graph.
    /// \invariant Does not contain `"` or `/` characters.
    std::string id;

    /// \brief Custom label for node. If empty, ::id will be used instead.
    std::optional<std::string> label;
};

/// \brief Defines directed edge of a Graph.
struct Edge
{
    /// \brief Specifies node, from which edge goes, either by absolute or relative path.
    /// \details See more about node paths in Graph documentation.
    std::string from;

    /// \brief Specifies node, to which edge goes, either by absolute or relative path.
    /// \details See more about node paths in Graph documentation.
    std::string to;
};

/// \brief Defines whole graph or subgraph of another graph.
///
/// \par Referencing nodes
/// \parblock
/// Nodes are organized into filesystem-like structure to make node referencing in complex graphs with lots of nesting
/// easier. Graphs and subgraphs have similar role to directories, where Graph::id is directory name. Nodes have similar
/// role to files, where Node::id is file name. Therefore any node can be referenced by relative or absolute path,
/// for example:
///
/// ```c++
/// // Referencing by relative path.
/// Graph {"complex_graph",
///        {},
///        {{"subgraph",
///          {},
///          {},
///          {{"a", {}}, {"b", {}}},
///          {}}},
///        {{"a", {}}, {"b", {}}},
///        {{"a", "subgraph/a"}, {"b", "subgraph/b"}}};
///
/// // Referencing by absolute path.
/// Graph {"complex_graph",
///         {},
///         {
///             {"subgraph",
///              {},
///              {},
///              {{"a", {}}, {"b", {}}},
///              {{"complex_graph/a", "a"}, {"complex_graph/b", "b"}}},
///         },
///         {{"a", {}}, {"b", {}}},
///         {}};
/// ```
/// \endparblock
struct Graph
{
    /// \brief Graph local id.
    /// \invariant Unique among other subgraphs, that belong to the same parent.
    /// \invariant Does not contain `"` or `/` characters.
    std::string id;

    /// \brief Custom label for graph. If empty, ::id will be used instead.
    std::optional<std::string> label;

    /// \brief Child graphs, that should be drawn as internal clusters.
    std::vector<Graph> subgraphs;

    /// \brief Nodes of this graph.
    std::vector<Node> nodes;

    /// \brief Edges, declared by this graph.
    /// \warning Other graphs can add edges with nodes from this graph using relative or absolute node paths.
    std::vector<Edge> edges;
};
} // namespace Emergence::VisualGraph
