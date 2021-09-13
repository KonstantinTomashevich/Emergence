#pragma once

#include <RecordCollection/Collection.hpp>

#include <Visual/Graph.hpp>

namespace Emergence::RecordCollection::Visualization
{
/// \brief Creates visual graph from given collection.
/// \details
/// - Guaranteed to have COLLECTION_ROOT_NODE.
/// - Guaranteed to have nodes for every child representation.
VisualGraph::Graph GraphFromCollection (const Collection &_collection);

/// \brief Creates visual graph from given representation.
VisualGraph::Graph GraphFromLinearRepresentation (const LinearRepresentation &_representation);

/// \brief Creates visual graph from given representation.
VisualGraph::Graph GraphFromPointRepresentation (const PointRepresentation &_representation);

/// \brief Creates visual graph from given representation.
VisualGraph::Graph GraphFromVolumetricRepresentation (const VolumetricRepresentation &_representation);

/// \return Id of a graph, that could be created from given collection.
std::string GraphId (const Collection &_collection);

/// \return Id of a graph, that could be created from given representation.
std::string GraphId (const LinearRepresentation &_representation);

/// \return Id of a graph, that could be created from given representation.
std::string GraphId (const PointRepresentation &_representation);

/// \return Id of a graph, that could be created from given representation.
std::string GraphId (const VolumetricRepresentation &_representation);
} // namespace Emergence::RecordCollection::Visualization
