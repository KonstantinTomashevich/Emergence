#pragma once

#include <ExportGraphApi.hpp>

#include <ostream>

#include <Visual/Graph.hpp>

namespace Emergence::Export::Graph
{
/// \brief Export given graph to given output stream.
/// \return If export succeeded.
/// \warning If exporter is unable to find `from` or `to` Node for given Edge, this Edge will not be exported.
ExportGraphApi bool Export (const VisualGraph::Graph &_graph, std::ostream &_output) noexcept;
} // namespace Emergence::Export::Graph
