#include <Export/Graph/Test/Expectation.hpp>

namespace Emergence::Export::Graph::Test::Expectation
{
Container::String TwoConnectedNodes ()
{
    return "digraph \"graph\" {\n"
           "    label=\"graph\";\n"
           "    \"graph/a\" [label=\"a\" ];\n"
           "    \"graph/b\" [label=\"b\" ];\n"
           "    \"graph/a\" -> \"graph/b\" [];\n"
           "}\n";
}

Container::String TwoConnectedNodesAndLabels ()
{
    return "digraph \"graph\" {\n"
           "    label=\"Cool Graph\";\n"
           "    \"graph/a\" [label=\"Node A\" ];\n"
           "    \"graph/b\" [label=\"Node B\" ];\n"
           "    \"graph/a\" -> \"graph/b\" [];\n"
           "}\n";
}

Container::String TwoSeparateSubgraphs ()
{
    return "digraph \"complex_graph\" {\n"
           "    label=\"complex_graph\";\n"
           "    subgraph \"cluster_complex_graph/first_subgraph\" {\n"
           "        label=\"first_subgraph\";\n"
           "        \"complex_graph/first_subgraph/a\" [label=\"a\" ];\n"
           "        \"complex_graph/first_subgraph/b\" [label=\"b\" ];\n"
           "    }\n"
           "    subgraph \"cluster_complex_graph/second_subgraph\" {\n"
           "        label=\"second_subgraph\";\n"
           "        \"complex_graph/second_subgraph/a\" [label=\"a\" ];\n"
           "        \"complex_graph/second_subgraph/b\" [label=\"b\" ];\n"
           "    }\n"
           "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/first_subgraph/b\" [];\n"
           "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
           "}\n";
}

Container::String TwoInterconnectedSubgraphs ()
{
    return "digraph \"complex_graph\" {\n"
           "    label=\"complex_graph\";\n"
           "    subgraph \"cluster_complex_graph/first_subgraph\" {\n"
           "        label=\"first_subgraph\";\n"
           "        \"complex_graph/first_subgraph/a\" [label=\"a\" ];\n"
           "        \"complex_graph/first_subgraph/b\" [label=\"b\" ];\n"
           "    }\n"
           "    subgraph \"cluster_complex_graph/second_subgraph\" {\n"
           "        label=\"second_subgraph\";\n"
           "        \"complex_graph/second_subgraph/a\" [label=\"a\" ];\n"
           "        \"complex_graph/second_subgraph/b\" [label=\"b\" ];\n"
           "    }\n"
           "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/first_subgraph/b\" [];\n"
           "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
           "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
           "}\n";
}

Container::String RelativePathsFromRoot ()
{
    return "digraph \"complex_graph\" {\n"
           "    label=\"complex_graph\";\n"
           "    subgraph \"cluster_complex_graph/subgraph\" {\n"
           "        label=\"subgraph\";\n"
           "        \"complex_graph/subgraph/a\" [label=\"a\" ];\n"
           "        \"complex_graph/subgraph/b\" [label=\"b\" ];\n"
           "    }\n"
           "    \"complex_graph/a\" [label=\"a\" ];\n"
           "    \"complex_graph/b\" [label=\"b\" ];\n"
           "    \"complex_graph/subgraph/a\" -> \"complex_graph/subgraph/b\" [];\n"
           "    \"complex_graph/a\" -> \"complex_graph/subgraph/a\" [];\n"
           "    \"complex_graph/b\" -> \"complex_graph/subgraph/b\" [];\n"
           "}\n";
}

Container::String DoubleSubgraphNesting ()
{
    return "digraph \"complex_graph\" {\n"
           "    label=\"complex_graph\";\n"
           "    subgraph \"cluster_complex_graph/first_subgraph\" {\n"
           "        label=\"first_subgraph\";\n"
           "        subgraph \"cluster_complex_graph/first_subgraph/first_subgraph\" {\n"
           "            label=\"first_subgraph\";\n"
           "            \"complex_graph/first_subgraph/first_subgraph/a\" [label=\"a\" ];\n"
           "            \"complex_graph/first_subgraph/first_subgraph/b\" [label=\"b\" ];\n"
           "        }\n"
           "        subgraph \"cluster_complex_graph/first_subgraph/second_subgraph\" {\n"
           "            label=\"second_subgraph\";\n"
           "            \"complex_graph/first_subgraph/second_subgraph/a\" [label=\"a\" ];\n"
           "            \"complex_graph/first_subgraph/second_subgraph/b\" [label=\"b\" ];\n"
           "        }\n"
           "    }\n"
           "    subgraph \"cluster_complex_graph/second_subgraph\" {\n"
           "        label=\"second_subgraph\";\n"
           "        subgraph \"cluster_complex_graph/second_subgraph/subgraph\" {\n"
           "            label=\"subgraph\";\n"
           "            \"complex_graph/second_subgraph/subgraph/a\" [label=\"a\" ];\n"
           "            \"complex_graph/second_subgraph/subgraph/b\" [label=\"b\" ];\n"
           "        }\n"
           "        \"complex_graph/second_subgraph/a\" [label=\"a\" ];\n"
           "        \"complex_graph/second_subgraph/b\" [label=\"b\" ];\n"
           "    }\n"
           "    \"complex_graph/a\" [label=\"a\" ];\n"
           "    \"complex_graph/b\" [label=\"b\" ];\n"
           "    \"complex_graph/first_subgraph/first_subgraph/a\" -> \"complex_graph/first_subgraph/first_subgraph/b\" "
           "[];\n"
           "    \"complex_graph/first_subgraph/second_subgraph/a\" -> "
           "\"complex_graph/first_subgraph/second_subgraph/b\" [];\n"
           "    \"complex_graph/first_subgraph/first_subgraph/a\" -> "
           "\"complex_graph/first_subgraph/second_subgraph/b\" "
           "[];\n"
           "    \"complex_graph/second_subgraph/subgraph/a\" -> \"complex_graph/second_subgraph/subgraph/b\" [];\n"
           "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/subgraph/a\" [];\n"
           "    \"complex_graph/second_subgraph/b\" -> \"complex_graph/second_subgraph/subgraph/b\" [];\n"
           "    \"complex_graph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
           "    \"complex_graph/b\" -> \"complex_graph/second_subgraph/subgraph/b\" [];\n"
           "    \"complex_graph/first_subgraph/first_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
           "}\n";
}

Container::String EdgeWithAbsolutePath ()
{
    return "digraph \"complex_graph\" {\n"
           "    label=\"complex_graph\";\n"
           "    subgraph \"cluster_complex_graph/first_subgraph\" {\n"
           "        label=\"first_subgraph\";\n"
           "        \"complex_graph/first_subgraph/a\" [label=\"a\" ];\n"
           "        \"complex_graph/first_subgraph/b\" [label=\"b\" ];\n"
           "    }\n"
           "    subgraph \"cluster_complex_graph/second_subgraph\" {\n"
           "        label=\"second_subgraph\";\n"
           "        \"complex_graph/second_subgraph/a\" [label=\"a\" ];\n"
           "        \"complex_graph/second_subgraph/b\" [label=\"b\" ];\n"
           "    }\n"
           "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/first_subgraph/b\" [];\n"
           "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
           "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/b\" [];\n"
           "}\n";
}

Container::String EdgeWithColor ()
{
    return "digraph \"graph\" {\n"
           "    label=\"graph\";\n"
           "    \"graph/a\" [label=\"a\" ];\n"
           "    \"graph/b\" [label=\"b\" ];\n"
           "    \"graph/a\" -> \"graph/b\" [color=\"#FF0000FF\" ];\n"
           "}\n";
}
} // namespace Emergence::Export::Graph::Test::Expectation
