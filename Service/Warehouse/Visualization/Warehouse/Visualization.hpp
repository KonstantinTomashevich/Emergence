#pragma once

#include <Warehouse/Registry.hpp>

namespace Emergence::Warehouse::Visualization
{
/// \brief Creates visual graph from given registry.
VisualGraph::Graph GraphFromRegistry (const Registry &_registry);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const FetchAscendingRangeQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const FetchDescendingRangeQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const FetchRayIntersectionQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const FetchSequenceQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const FetchShapeIntersectionQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const FetchSingletonQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const FetchValueQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const InsertLongTermQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const InsertShortTermQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const ModifyAscendingRangeQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const ModifyDescendingRangeQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const ModifyRayIntersectionQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const ModifySequenceQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const ModifyShapeIntersectionQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const ModifySingletonQuery &_query);

/// \brief Creates visual graph from given query.
VisualGraph::Graph GraphFromQuery (const ModifyValueQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const FetchAscendingRangeQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const FetchDescendingRangeQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const FetchRayIntersectionQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const FetchSequenceQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const FetchShapeIntersectionQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const FetchSingletonQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const FetchValueQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const InsertLongTermQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const InsertShortTermQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const ModifyAscendingRangeQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const ModifyDescendingRangeQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const ModifyRayIntersectionQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const ModifySequenceQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const ModifyShapeIntersectionQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const ModifySingletonQuery &_query);

/// \return Id of a graph, that could be created from given query.
std::string GraphId (const ModifyValueQuery &_query);
} // namespace Emergence::Warehouse::Visualization
