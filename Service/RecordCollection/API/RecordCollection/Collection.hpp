#pragma once

#include <vector>

#include <StandardLayout/Mapping.hpp>

#include <RecordCollection/LinearResolver.hpp>
#include <RecordCollection/PointResolver.hpp>
#include <RecordCollection/VolumetricResolver.hpp>

namespace Emergence::RecordCollection
{
/// \brief Stores records of the same type and provides fast lookup using resolvers.
class Collection final
{
public:
    /// \brief Wraps insertion of several records into one transaction.
    class Inserter final
    {
    public:
        Inserter (const Inserter &_other) = delete;

        Inserter (Inserter &&_other);

        /// \brief Inserts all allocated records into collection and ends transaction.
        ~Inserter ();

        /// \brief Allocates new record for insertion.
        /// \warning Record type is unknown during compile time, therefore
        ///          appropriate constructor should be called after allocation.
        void *AllocateRecord ();

    private:
        /// Collection constructs inserters.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit Inserter (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over Collection linear resolvers.
    ///
    /// \warning Collection::CreateLinearResolver invalidates these iterators.
    class LinearResolverIterator final
    {
    public:
        ~LinearResolverIterator () noexcept;

        /// \return LinearResolver, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        LinearResolver operator * () const noexcept;

        /// \brief Move to next field.
        /// \invariant Inside valid bounds, but not in the ending.
        LinearResolverIterator &operator ++ () noexcept;

        /// \brief Move to next field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        LinearResolverIterator operator ++ (int) noexcept;

        /// \brief Move to previous field.
        /// \invariant Inside valid bounds, but not in the beginning.
        LinearResolverIterator &operator -- () noexcept;

        /// \brief Move to previous field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the beginning.
        LinearResolverIterator operator -- (int) noexcept;

        bool operator == (const LinearResolverIterator &_other) const noexcept;

        bool operator != (const LinearResolverIterator &_other) const noexcept;

    private:
        /// Collection constructs iterators for linear resolvers.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit LinearResolverIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over Collection point resolvers.
    ///
    /// \warning Collection::CreatePointResolver invalidates these iterators.
    class PointResolverIterator final
    {
    public:
        ~PointResolverIterator () noexcept;

        /// \return PointResolver, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        PointResolver operator * () const noexcept;

        /// \brief Move to next field.
        /// \invariant Inside valid bounds, but not in the ending.
        PointResolverIterator &operator ++ () noexcept;

        /// \brief Move to next field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        PointResolverIterator operator ++ (int) noexcept;

        /// \brief Move to previous field.
        /// \invariant Inside valid bounds, but not in the beginning.
        PointResolverIterator &operator -- () noexcept;

        /// \brief Move to previous field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the beginning.
        PointResolverIterator operator -- (int) noexcept;

        bool operator == (const PointResolverIterator &_other) const noexcept;

        bool operator != (const PointResolverIterator &_other) const noexcept;

    private:
        /// Collection constructs iterators for point resolvers.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit PointResolverIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over Collection volumetric resolvers.
    ///
    /// \warning Collection::CreateVolumetricResolver invalidates these iterators.
    class VolumetricResolverIterator final
    {
    public:
        ~VolumetricResolverIterator () noexcept;

        /// \return VolumetricResolver, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        VolumetricResolver operator * () const noexcept;

        /// \brief Move to next field.
        /// \invariant Inside valid bounds, but not in the ending.
        VolumetricResolverIterator &operator ++ () noexcept;

        /// \brief Move to next field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        VolumetricResolverIterator operator ++ (int) noexcept;

        /// \brief Move to previous field.
        /// \invariant Inside valid bounds, but not in the beginning.
        VolumetricResolverIterator &operator -- () noexcept;

        /// \brief Move to previous field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the beginning.
        VolumetricResolverIterator operator -- (int) noexcept;

        bool operator == (const VolumetricResolverIterator &_other) const noexcept;

        bool operator != (const VolumetricResolverIterator &_other) const noexcept;

    private:
        /// Collection constructs iterators for volumetric resolvers.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit VolumetricResolverIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Describes one of the VolumetricResolver dimensions, used for VolumetricResolver creation.
    struct DimensionDescription
    {
        /// \brief Pointer to minimum possible value of #minBorderField.
        ///
        /// \details Values, that are less than this value will be processed as this value.
        ///          Guarantied to be copied during resolver creation, therefore can point to stack memory.
        const void *globalMinBorder;

        /// \brief Id of field, that holds record minimum border value for this dimension.
        ///
        /// \invariant Field archetype is FieldArchetype::INT, FieldArchetype::UINT or FieldArchetype::FLOAT.
        StandardLayout::FieldId minBorderField;

        /// \brief Pointer to maximum possible value of #maxBorderField.
        ///
        /// \details Values, that are greater than this value will be processed as this value.
        ///          Guarantied to be copied during resolver creation, therefore can point to stack memory.
        const void *globalMaxBorder;

        /// \brief Id of field, that holds record maximum border value for this dimension.
        ///
        /// \invariant Field archetype is FieldArchetype::INT, FieldArchetype::UINT or FieldArchetype::FLOAT.
        StandardLayout::FieldId maxBorderField;
    };

    /// \brief Construct collection for records of the type, which fields are described in given _typeMapping.
    explicit Collection (StandardLayout::Mapping _typeMapping);

    /// Collections are designed to store lots of records, therefore it's not optimal to copy such collections.
    Collection (const Collection &_other) = delete;

    Collection (Collection &&_other) noexcept;

    ~Collection () noexcept;

    /// \brief Starts insertion transaction.
    /// \invariant There is no active insertion transactions in this collection and cursors in its resolvers.
    Inserter Insert () noexcept;

    /// \brief Adds LinearResolver to Collection, which sorts records by value of given _keyField.
    /// \invariant There is no active insertion transactions in this collection and cursors in its resolvers.
    LinearResolver CreateLinearResolver (StandardLayout::FieldId _keyField) const noexcept;

    /// \brief Adds PointResolver to Collection, that uses given _keyFields as point position.
    /// \invariant There is no active insertion transactions in this collection and cursors in its resolvers.
    PointResolver CreatePointResolver (const std::vector <StandardLayout::FieldId> _keyFields) const noexcept;

    /// \brief Adds VolumetricResolver to Collection, that uses given _dimensions.
    /// \invariant There is no active insertion transactions in this collection and cursors in its resolvers.
    VolumetricResolver CreateVolumetricResolver (const std::vector <DimensionDescription> _dimensions) const noexcept;

    /// \return Iterator, that points to beginning of linear resolvers range.
    LinearResolverIterator LinearResolverBegin () noexcept;

    /// \return Iterator, that points to ending of linear resolvers range.
    LinearResolverIterator LinearResolverEnd () noexcept;

    /// \return Iterator, that points to beginning of point resolvers range.
    PointResolverIterator PointResolverBegin () noexcept;

    /// \return Iterator, that points to ending of point resolvers range.
    PointResolverIterator PointResolverEnd () noexcept;

    /// \return Iterator, that points to beginning of volumetric resolvers range.
    VolumetricResolverIterator VolumetricResolverBegin () noexcept;

    /// \return Iterator, that points to ending of volumetric resolvers range.
    VolumetricResolverIterator VolumetricResolverEnd () noexcept;

    Collection &operator = (const Collection &_other) = delete;

    Collection &operator = (Collection &&_other) noexcept;

private:
    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection
