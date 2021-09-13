#pragma once

#include <vector>

#include <Visual/Graph.hpp>

namespace Emergence::Warehouse::Test
{
struct VisualizationExpectedResult
{
    VisualGraph::Graph registry;
    std::vector<VisualGraph::Graph> queries;
};

namespace Graphs
{
VisualizationExpectedResult OneQueryForEachCombination ();

VisualizationExpectedResult Singletons ();

VisualizationExpectedResult Sequences ();

VisualizationExpectedResult LongTerms ();
} // namespace Visualization
} // namespace Emergence::Warehouse::Test
