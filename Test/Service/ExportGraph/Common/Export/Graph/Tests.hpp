#pragma once

#include <Visual/Graph.hpp>

namespace Emergence::Export::Graph::Test
{
VisualGraph::Graph TwoConnectedNodes () noexcept;

VisualGraph::Graph TwoConnectedNodesAndLabels () noexcept;

VisualGraph::Graph TwoSeparateSubgraphs () noexcept;

VisualGraph::Graph TwoInterconnectedSubgraphs () noexcept;

VisualGraph::Graph RelativePathsFromRoot () noexcept;

VisualGraph::Graph DoubleSubgraphNesting () noexcept;

VisualGraph::Graph EdgeWithAbsolutePath () noexcept;

VisualGraph::Graph WithDuplicateGraphIds () noexcept;

VisualGraph::Graph WithIncorrectGraphIds () noexcept;

VisualGraph::Graph WithDuplicateNodeIds () noexcept;

VisualGraph::Graph WithIncorrectNodeIds () noexcept;
} // namespace Emergence::Export::Graph::Test
