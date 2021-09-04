#pragma once

#include <ostream>

#include <Visual/Graph.hpp>

namespace Emergence::Export::Graph
{
/// \brief Export given graph to given output stream.
/// \return If export succeeded.
bool Export (const VisualGraph::Graph &_graph, std::ostream &_output) noexcept;
} // namespace Emergence::Export::Graph
