#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Emergence::VisualGraph
{
struct Node
{
    std::string id;

    std::optional<std::string> label;
};

struct Edge
{
    std::string from;

    std::string to;
};

struct Graph
{
    std::string id;

    std::optional<std::string> label;

    std::vector<Graph> subgraphs;

    std::vector<Node> nodes;

    std::vector<Edge> edges;
};
} // namespace Emergence::VisualGraph
