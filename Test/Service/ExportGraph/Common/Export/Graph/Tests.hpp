#pragma once

#include <Visual/Graph.hpp>

namespace Emergence::Export::Graph::Test
{
VisualGraph::Graph TwoConnectedNodes () noexcept;

VisualGraph::Graph TwoConnectedNodesAndLabels () noexcept;

VisualGraph::Graph TwoSeparateSubgraphs () noexcept;

VisualGraph::Graph TwoInterconnectedSubgraphs () noexcept;

VisualGraph::Graph SubgraphAndRootNodes () noexcept;

VisualGraph::Graph DoubleSubgraphNesting () noexcept;

VisualGraph::Graph EdgeWithUpperScopeNode () noexcept;

VisualGraph::Graph WithDuplicateGraphIds () noexcept;

VisualGraph::Graph WithDuplicateNodeIds () noexcept;
} // namespace Emergence::Export::Graph::Test
