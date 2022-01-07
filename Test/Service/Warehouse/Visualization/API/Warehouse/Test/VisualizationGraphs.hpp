#pragma once

#include <Visual/Graph.hpp>

namespace Emergence::Warehouse::Test
{
struct VisualizationExpectedResult
{
    VisualGraph::Graph registry;
    Container::Vector<VisualGraph::Graph> queries;
};

namespace Graphs
{
VisualizationExpectedResult OneQueryForEachCombination ();

VisualizationExpectedResult Singletons ();

VisualizationExpectedResult Sequences ();

VisualizationExpectedResult LongTerms ();
} // namespace Graphs
} // namespace Emergence::Warehouse::Test
