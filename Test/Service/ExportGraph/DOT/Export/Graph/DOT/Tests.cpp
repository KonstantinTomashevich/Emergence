#include <sstream>

#include <Export/Graph.hpp>

#include <Export/Graph/Tests.hpp>

#include <Testing/SetupMain.hpp>
#include <Testing/Testing.hpp>

using namespace Emergence::Export;
using namespace Emergence::Export::Graph::Test;

BEGIN_SUITE (ExportGraph)

TEST_CASE (TwoConnectedNodes)
{
    std::stringstream stream;
    const bool exported = Graph::Export (TwoConnectedNodes (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"graph\" {\n"
        "    \"graph/a\" [];\n"
        "    \"graph/b\" [];\n"
        "    \"graph/a\" -> \"graph/b\";\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (TwoConnectedNodesAndLabels)
{
    std::stringstream stream;
    const bool exported = Graph::Export (TwoConnectedNodesAndLabels (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"graph\" {\n"
        "    label=\"Cool Graph\";\n"
        "    \"graph/a\" [label=\"Node A\" ];\n"
        "    \"graph/b\" [label=\"Node B\" ];\n"
        "    \"graph/a\" -> \"graph/b\";\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (TwoSeparateSubgraphs)
{
    std::stringstream stream;
    const bool exported = Graph::Export (TwoSeparateSubgraphs (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    subgraph \"cluster_first_subgraph\" {\n"
        "        \"complex_graph/first_subgraph/a\" [];\n"
        "        \"complex_graph/first_subgraph/b\" [];\n"
        "    }\n"
        "    subgraph \"cluster_second_subgraph\" {\n"
        "        \"complex_graph/second_subgraph/a\" [];\n"
        "        \"complex_graph/second_subgraph/b\" [];\n"
        "    }\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/first_subgraph/b\";\n"
        "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/b\";\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (TwoInterconnectedSubgraphs)
{
    std::stringstream stream;
    const bool exported = Graph::Export (TwoInterconnectedSubgraphs (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    subgraph \"cluster_first_subgraph\" {\n"
        "        \"complex_graph/first_subgraph/a\" [];\n"
        "        \"complex_graph/first_subgraph/b\" [];\n"
        "    }\n"
        "    subgraph \"cluster_second_subgraph\" {\n"
        "        \"complex_graph/second_subgraph/a\" [];\n"
        "        \"complex_graph/second_subgraph/b\" [];\n"
        "    }\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/first_subgraph/b\";\n"
        "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/b\";\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/second_subgraph/b\";\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (SubgraphAndRootNodes)
{
    std::stringstream stream;
    const bool exported = Graph::Export (SubgraphAndRootNodes (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    subgraph \"cluster_subgraph\" {\n"
        "        \"complex_graph/subgraph/a\" [];\n"
        "        \"complex_graph/subgraph/b\" [];\n"
        "    }\n"
        "    \"complex_graph/a\" [];\n"
        "    \"complex_graph/b\" [];\n"
        "    \"complex_graph/subgraph/a\" -> \"complex_graph/subgraph/b\";\n"
        "    \"complex_graph/a\" -> \"complex_graph/subgraph/a\";\n"
        "    \"complex_graph/b\" -> \"complex_graph/subgraph/b\";\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (DoubleSubgraphNesting)
{
    std::stringstream stream;
    const bool exported = Graph::Export (DoubleSubgraphNesting (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    subgraph \"cluster_first_subgraph\" {\n"
        "        subgraph \"cluster_first_subgraph\" {\n"
        "            \"complex_graph/first_subgraph/first_subgraph/a\" [];\n"
        "            \"complex_graph/first_subgraph/first_subgraph/b\" [];\n"
        "        }\n"
        "        subgraph \"cluster_second_subgraph\" {\n"
        "            \"complex_graph/first_subgraph/second_subgraph/a\" [];\n"
        "            \"complex_graph/first_subgraph/second_subgraph/b\" [];\n"
        "        }\n"
        "    }\n"
        "    subgraph \"cluster_second_subgraph\" {\n"
        "        subgraph \"cluster_subgraph\" {\n"
        "            \"complex_graph/second_subgraph/subgraph/a\" [];\n"
        "            \"complex_graph/second_subgraph/subgraph/b\" [];\n"
        "        }\n"
        "        \"complex_graph/second_subgraph/a\" [];\n"
        "        \"complex_graph/second_subgraph/b\" [];\n"
        "    }\n"
        "    \"complex_graph/a\" [];\n"
        "    \"complex_graph/b\" [];\n"
        "    \"complex_graph/first_subgraph/first_subgraph/a\" -> \"complex_graph/first_subgraph/first_subgraph/b\";\n"
        "    \"complex_graph/first_subgraph/second_subgraph/a\" -> "
        "\"complex_graph/first_subgraph/second_subgraph/b\";\n"
        "    \"complex_graph/first_subgraph/first_subgraph/a\" -> \"complex_graph/first_subgraph/second_subgraph/b\";\n"
        "    \"complex_graph/second_subgraph/subgraph/a\" -> \"complex_graph/second_subgraph/subgraph/b\";\n"
        "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/subgraph/a\";\n"
        "    \"complex_graph/second_subgraph/b\" -> \"complex_graph/second_subgraph/subgraph/b\";\n"
        "    \"complex_graph/a\" -> \"complex_graph/second_subgraph/b\";\n"
        "    \"complex_graph/b\" -> \"complex_graph/second_subgraph/subgraph/b\";\n"
        "    \"complex_graph/first_subgraph/first_subgraph/a\" -> \"complex_graph/second_subgraph/b\";\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (EdgeWithUpperScopeNode)
{
    std::stringstream stream;
    const bool exported = Graph::Export (EdgeWithUpperScopeNode (), stream);
    CHECK (exported);

    const std::string expected =
        "digraph \"complex_graph\" {\n"
        "    subgraph \"cluster_first_subgraph\" {\n"
        "        \"complex_graph/first_subgraph/a\" [];\n"
        "        \"complex_graph/first_subgraph/b\" [];\n"
        "    }\n"
        "    subgraph \"cluster_second_subgraph\" {\n"
        "        \"complex_graph/second_subgraph/a\" [];\n"
        "        \"complex_graph/second_subgraph/b\" [];\n"
        "    }\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/first_subgraph/b\";\n"
        "    \"complex_graph/first_subgraph/a\" -> \"complex_graph/second_subgraph/b\";\n"
        "    \"complex_graph/second_subgraph/a\" -> \"complex_graph/second_subgraph/b\";\n"
        "}\n";

    CHECK_EQUAL (expected, stream.str ());
}

TEST_CASE (WithDuplicateGraphIds)
{
    std::stringstream stream;
    const bool exported = Graph::Export (WithDuplicateGraphIds (), stream);
    CHECK (!exported);
}

TEST_CASE (WithDuplicateNodeIds)
{
    std::stringstream stream;
    const bool exported = Graph::Export (WithDuplicateNodeIds (), stream);
    CHECK (!exported);
}

END_SUITE
