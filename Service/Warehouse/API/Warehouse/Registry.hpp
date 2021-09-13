#pragma once

#include <vector>

#include <API/Common/ImplementationBinding.hpp>

#include <StandardLayout/Mapping.hpp>

#include <Warehouse/FetchAscendingRangeQuery.hpp>
#include <Warehouse/FetchDescendingRangeQuery.hpp>
#include <Warehouse/FetchRayIntersectionQuery.hpp>
#include <Warehouse/FetchSequenceQuery.hpp>
#include <Warehouse/FetchShapeIntersectionQuery.hpp>
#include <Warehouse/FetchSingletonQuery.hpp>
#include <Warehouse/FetchValueQuery.hpp>
#include <Warehouse/InsertLongTermQuery.hpp>
#include <Warehouse/InsertShortTermQuery.hpp>
#include <Warehouse/ModifyAscendingRangeQuery.hpp>
#include <Warehouse/ModifyDescendingRangeQuery.hpp>
#include <Warehouse/ModifyRayIntersectionQuery.hpp>
#include <Warehouse/ModifySequenceQuery.hpp>
#include <Warehouse/ModifyShapeIntersectionQuery.hpp>
#include <Warehouse/ModifySingletonQuery.hpp>
#include <Warehouse/ModifyValueQuery.hpp>

namespace Emergence::Warehouse
{
/// \brief Stores objects of different types and provides prepared queries, that allow to interact with this objects.
///
/// \par Operation complexity
/// \parblock
/// Registry operations could be quite complex and time consuming: they may involve object storage allocations and
/// query indexing backend setup. Therefore it's advised to prepare all required queries before gameplay starts
/// (for example, during level loading). Preparing queries during each frame update could lead to performance drops.
/// \endparblock
///
/// \par Resource sharing guaranties
/// \parblock
/// - There is always not more than one object storage per object type.
/// - If there are multiple prepared queries for one object type, they share object storage.
/// - If there are multiple equal prepared queries, they share query indexing backend. Prepared queries are equal if
///   they work with the same object type and have equal types and parameters.
/// \endparblock
///
/// \par Prepared query combinations
/// \parblock
/// Not all query combinations for single object type are supported:
/// - Some of them are just logically incorrect. For example FetchSingletonQuery + FetchSequenceQuery.
/// - Some of them don't have reasonable use cases and are difficult to implement without decreasing performance
///   and increasing resource use. For example FetchSequenceQuery + FetchShapeIntersectionQuery.
///
/// Warehouse provides three categories, in which any query combination is guaranteed to be supported:
/// - Singleton queries:
///     - FetchSingletonQuery
///     - ModifySingletonQuery
/// - Optimized for objects with short lifetime, like events:
///     - InsertShortTermQuery.
///     - FetchSequenceQuery.
///     - ModifySequenceQuery.
/// - Optimized for objects with long lifetime, like any world object:
///     - InsertLongTermQuery.
///     - FetchValueQuery.
///     - ModifyValueQuery.
///     - FetchAscendingRangeQuery.
///     - ModifyAscendingRangeQuery.
///     - FetchDescendingRangeQuery.
///     - ModifyDescendingRangeQuery.
///     - FetchShapeIntersectionQuery.
///     - ModifyShapeIntersectionQuery.
///     - FetchRayIntersectionQuery.
///     - ModifyRayIntersectionQuery.
///
/// Implementations are allowed to provide additional categories.
/// \endparblock
///
/// \par Garbage collection
/// \parblock
/// If there is no prepared queries for object type, object storage will be automatically deallocated.
/// Destructors will not be called, because object type is unknown during compile time. If objects require
/// non-trivial destruction, user should take care of that.
/// \endparblock
///
/// \par Singletons
/// \parblock
/// Singleton type is unknown during compile time, therefore singleton constructor will not be called. Instead of
/// construction, singleton memory block will be filled with zeros. If singleton requires additional initialization,
/// ModifySingletonQuery must be used.
/// \endparblock
class Registry final
{
public:
    Registry (std::string _name) noexcept;

    /// Registry holds lots of objects, therefore it's not optimal to copy it.
    Registry (const Registry &_other) = delete;

    Registry (Registry &&_other) noexcept;

    /// \invariant There is no prepared query instances that belong to this registry.
    ~Registry () noexcept;

    /// \brief Prepare FetchSingletonQuery for given type.
    FetchSingletonQuery FetchSingleton (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Prepare ModifySingletonQuery for given type.
    ModifySingletonQuery ModifySingleton (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Prepare InsertShortTermQuery for given type.
    InsertShortTermQuery InsertShortTerm (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Prepare FetchSequenceQuery for given type.
    FetchSequenceQuery FetchSequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Prepare ModifySequenceQuery for given type.
    ModifySequenceQuery ModifySequence (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Prepare InsertLongTermQuery for given type.
    InsertLongTermQuery InsertLongTerm (const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Prepare FetchValueQuery for given type on given key fields.
    /// \invariant There is at least one key field.
    FetchValueQuery FetchValue (const StandardLayout::Mapping &_typeMapping,
                                const std::vector<StandardLayout::FieldId> &_keyFields) noexcept;

    /// \brief Prepare ModifyValueQuery for given type on given key fields.
    /// \invariant There is at least one key field.
    ModifyValueQuery ModifyValue (const StandardLayout::Mapping &_typeMapping,
                                  const std::vector<StandardLayout::FieldId> &_keyFields) noexcept;

    /// \brief Prepare FetchAscendingRangeQuery for given type on given key field.
    FetchAscendingRangeQuery FetchAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                  StandardLayout::FieldId _keyField) noexcept;

    /// \brief Prepare ModifyAscendingRangeQuery for given type on given key field.
    ModifyAscendingRangeQuery ModifyAscendingRange (const StandardLayout::Mapping &_typeMapping,
                                                    StandardLayout::FieldId _keyField) noexcept;

    /// \brief Prepare FetchDescendingRangeQuery for given type on given key field.
    FetchDescendingRangeQuery FetchDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                    StandardLayout::FieldId _keyField) noexcept;

    /// \brief Prepare ModifyDescendingRangeQuery for given type on given key field.
    ModifyDescendingRangeQuery ModifyDescendingRange (const StandardLayout::Mapping &_typeMapping,
                                                      StandardLayout::FieldId _keyField) noexcept;

    /// \brief Prepare FetchShapeIntersectionQuery for given type on given dimensions.
    /// \invariant There is at least one dimension.
    FetchShapeIntersectionQuery FetchShapeIntersection (const StandardLayout::Mapping &_typeMapping,
                                                        const std::vector<Dimension> &_dimensions) noexcept;

    /// \brief Prepare ModifyShapeIntersectionQuery for given type on given dimensions.
    /// \invariant There is at least one dimension.
    ModifyShapeIntersectionQuery ModifyShapeIntersection (const StandardLayout::Mapping &_typeMapping,
                                                          const std::vector<Dimension> &_dimensions) noexcept;

    /// \brief Prepare FetchRayIntersectionQuery for given type on given dimensions.
    /// \invariant There is at least one dimension.
    FetchRayIntersectionQuery FetchRayIntersection (const StandardLayout::Mapping &_typeMapping,
                                                    const std::vector<Dimension> &_dimensions) noexcept;

    /// \brief Prepare ModifyRayIntersectionQuery for given type on given dimensions.
    /// \invariant There is at least one dimension.
    ModifyRayIntersectionQuery ModifyRayIntersection (const StandardLayout::Mapping &_typeMapping,
                                                      const std::vector<Dimension> &_dimensions) noexcept;

    /// \return Name of this registry, that can be used for debug or visualization purposes.
    const std::string GetName () const noexcept;

    /// \brief Utility for Warehouse::Visualization, that allows implementation to add custom content to graphs.
    void AddCustomVisualization (VisualGraph::Graph &_graph) const noexcept;

    /// Registry holds lots of objects, therefore it's not optimal to copy assign it.
    Registry &operator= (const Registry &_other) = delete;

    Registry &operator= (Registry &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Warehouse
