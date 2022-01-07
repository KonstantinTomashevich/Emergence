#pragma once

#include <ostream>
#include <string>

#include <Container/HashSet.hpp>
#include <Container/Optional.hpp>
#include <Container/Vector.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::Graph::DOTExporter
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
    Container::Optional<Container::HashSet<std::string>> Process (const VisualGraph::Graph &_graph,
                                                                  std::string _pathPrefix = "",
                                                                  const std::string &_outerIndentation = "");

    std::ostream &output;

    /// \details DOT creates nodes on demand, therefore referencing node by the absolute path before it was declared in
    ///          its subgraph will result in node creation in incorrect subgraph. We use the simplest solution for this
    ///          problem: all edges (doesn't matter in which graph edge was declared) are defined in root graph after
    ///          all subgraphs and nodes. Therefore we store all resolved edges in this array until all subgraphs and
    ///          nodes are defined.
    Container::Vector<VisualGraph::Edge> resolvedEdges;
};
} // namespace Emergence::Graph::DOTExporter
