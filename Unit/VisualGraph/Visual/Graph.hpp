#pragma once

#include <VisualGraphApi.hpp>

#include <Container/Optional.hpp>
#include <Container/String.hpp>
#include <Container/Vector.hpp>

namespace Emergence::VisualGraph
{
/// \brief Id separator for relative and absolute node paths.
/// \details See more about node paths in Graph documentation.
constexpr char NODE_PATH_SEPARATOR = '/';

/// \brief Defines node of a Graph.
struct VisualGraphApi Node
{
    /// \brief Node local id in its graph.
    /// \invariant Unique among nodes of parent graph.
    /// \invariant Does not contain `"` or NODE_PATH_SEPARATOR characters.
    Container::String id;

    /// \brief Custom label for node. If empty, ::id will be used instead.
    Container::Optional<Container::String> label;

    bool operator== (const Node &_other) const = default;
};

/// \brief Defines directed edge of a Graph.
struct VisualGraphApi Edge
{
    /// \brief Specifies node, from which edge goes, either by absolute or relative path.
    /// \details See more about node paths in Graph documentation.
    Container::String from;

    /// \brief Specifies node, to which edge goes, either by absolute or relative path.
    /// \details See more about node paths in Graph documentation.
    Container::String to;

    /// \brief Specifies custom fill color for this edge.
    Container::Optional<Container::String> color;

    bool operator== (const Edge &_other) const = default;
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
struct VisualGraphApi Graph
{
    /// \brief Graph local id.
    /// \invariant Unique among other subgraphs, that belong to the same parent.
    /// \invariant Does not contain `"` or NODE_PATH_SEPARATOR characters.
    Container::String id;

    /// \brief Custom label for graph. If empty, ::id will be used instead.
    Container::Optional<Container::String> label;

    /// \brief Child graphs, that should be drawn as internal clusters.
    Container::Vector<Graph> subgraphs;

    /// \brief Nodes of this graph.
    Container::Vector<Node> nodes;

    /// \brief Edges, declared by this graph.
    /// \warning Other graphs can add edges with nodes from this graph using relative or absolute node paths.
    Container::Vector<Edge> edges;

    bool operator== (const Graph &_other) const = default;
};

VisualGraphApi Memory::Profiler::AllocationGroup GetDefaultAllocationGroup () noexcept;

/// \brief Contains common constants for graphs, that are created from Emergence libraries and services.
namespace Common::Constants
{
/// \brief Root graph should always have this id.
constexpr const char *DEFAULT_ROOT_GRAPH_ID = ".";

/// \brief Graphs for all StandardLayout::Mapping's should be subgraphs of graph with this name.
///        And graph with this name should be subgraph of the root graph.
constexpr const char *MAPPING_SUBGRAPH = "Mapping";

/// \brief StandardLayout::Mapping graph should have node with this name, that has edges with all direct fields.
constexpr const char *MAPPING_ROOT_NODE = ".";

/// \brief Default color for edges, that connect mapping users with mapping nodes.
/// \details This edges should have special color, otherwise they will blend in with mapping structure edges.
constexpr const char *MAPPING_USAGE_COLOR = "#3F48FEFF";

/// \brief RecordCollection::Collection graph is guaranteed to have node
///        with this name, that will be used as root for its graph.
constexpr const char *RECORD_COLLECTION_ROOT_NODE = ".";

/// \brief RecordCollection representation graph is guaranteed to have node
///        with this name, that will be used as root for its graph.
constexpr const char *RECORD_COLLECTION_REPRESENTATION_ROOT_NODE = ".";

/// \brief Graphs for all Warehouse::Registry's should be subgraphs of graph with this name.
///        And graph with this name should be subgraph of the root graph.
constexpr const char *WAREHOUSE_REGISTRY_SUBGRAPH = "Registry";

/// \brief Warehouse::Registry graph is guaranteed to have node with this name, that will be used as root for its graph.
constexpr const char *WAREHOUSE_REGISTRY_ROOT_NODE = ".";

/// \brief Warehouse query graph is guaranteed to have node with this name, that will be used as root for its graph.
constexpr const char *WAREHOUSE_QUERY_ROOT_NODE = ".";
} // namespace Common::Constants
} // namespace Emergence::VisualGraph

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (VisualGraphApi, Emergence::VisualGraph::Node)
EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (VisualGraphApi, Emergence::VisualGraph::Edge)
EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (VisualGraphApi, Emergence::VisualGraph::Graph)
