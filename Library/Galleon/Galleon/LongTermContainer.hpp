#pragma once

#include <cassert>

#include <API/Common/Cursor.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Galleon/ContainerBase.hpp>

#include <RecordCollection/Collection.hpp>
#include <RecordCollection/LinearRepresentation.hpp>
#include <RecordCollection/PointRepresentation.hpp>
#include <RecordCollection/VolumetricRepresentation.hpp>

namespace Emergence::Galleon
{
/// \brief Storage for objects that are created and destroyed rarely.
/// \details Prepared queries for this container use RecordCollection to provide fast parametrized lookups.
class LongTermContainer final : public ContainerBase
{
public:
    /// \brief Prepared query, used to start insertion transactions.
    class InsertQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept = default;

            /// \invariant Previously allocated object must be initialized before cursor destruction.
            ~Cursor () noexcept = default;

            /// \return Pointer to memory, allocated for the new object.
            /// \invariant Previously allocated object must be initialized before next call.
            void *operator ++ () noexcept;

            /// Assigning cursors looks counter intuitive.
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);

        private:
            friend class InsertQuery;

            explicit Cursor (Handling::Handle <LongTermContainer> _container) noexcept;

            Handling::Handle <LongTermContainer> container;
            RecordCollection::Collection::Allocator allocator;
        };

        InsertQuery (const InsertQuery &_other) noexcept = default;

        InsertQuery (InsertQuery &&_other) noexcept = default;

        ~InsertQuery () noexcept = default;

        Cursor Execute () const noexcept;

        Handling::Handle <LongTermContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (InsertQuery);

    private:
        friend class LongTermContainer;

        explicit InsertQuery (Handling::Handle <LongTermContainer> _container) noexcept;

        Handling::Handle <LongTermContainer> container;
    };

    // TODO: For now we plainly use RecordCollection cursors and query formats. Is it ok or is it bad?

    template <typename Representation>
    class RepresentationQueryBase
    {
    public:
        RepresentationQueryBase (const RepresentationQueryBase &_other) noexcept = default;

        RepresentationQueryBase (RepresentationQueryBase &&_other) noexcept = default;

        ~RepresentationQueryBase () noexcept;

        Handling::Handle <LongTermContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (RepresentationQueryBase);

    protected:
        RepresentationQueryBase (Handling::Handle <LongTermContainer> _container,
                                 Representation _representation) noexcept;

        Handling::Handle <LongTermContainer> container;
        Representation representation;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ///        each key field value is equal to according value in given values sequence.
    class FetchValueQuery final : public RepresentationQueryBase <RecordCollection::PointRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::PointRepresentation::ReadCursor;

        Cursor Execute (RecordCollection::PointRepresentation::Point _values) noexcept;

        RecordCollection::PointRepresentation::KeyFieldIterator KeyFieldBegin () const noexcept;

        RecordCollection::PointRepresentation::KeyFieldIterator KeyFieldEnd () const noexcept;

    private:
        friend class LongTermContainer;

        FetchValueQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::PointRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ///        each key field value is equal to according value in given values sequence.
    class ModifyValueQuery final : public RepresentationQueryBase <RecordCollection::PointRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::PointRepresentation::EditCursor;

        Cursor Execute (RecordCollection::PointRepresentation::Point _values) noexcept;

        RecordCollection::PointRepresentation::KeyFieldIterator KeyFieldBegin () const noexcept;

        RecordCollection::PointRepresentation::KeyFieldIterator KeyFieldEnd () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyValueQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::PointRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ///        given min max interval contains key field value.
    class FetchRangeQuery final : public RepresentationQueryBase <RecordCollection::LinearRepresentation>
    {
    public:
        /// \details Objects are sorted in ascending order on key field value.
        using Cursor = RecordCollection::LinearRepresentation::AscendingReadCursor;

        Cursor Execute (
            RecordCollection::LinearRepresentation::KeyFieldValue _min,
            RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept;

        StandardLayout::Field GetKeyField () const noexcept;

    private:
        friend class LongTermContainer;

        FetchRangeQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::LinearRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       given min max interval contains key field value.
    class ModifyRangeQuery final : public RepresentationQueryBase <RecordCollection::LinearRepresentation>
    {
    public:
        /// \details Objects are sorted in ascending order on key field value.
        using Cursor = RecordCollection::LinearRepresentation::AscendingEditCursor;

        Cursor Execute (
            RecordCollection::LinearRepresentation::KeyFieldValue _min,
            RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept;

        StandardLayout::Field GetKeyField () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyRangeQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::LinearRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ///        given min max interval contains key field value.
    class FetchReversedRangeQuery final : public RepresentationQueryBase <RecordCollection::LinearRepresentation>
    {
    public:
        /// \details Objects are sorted in descending order on key field value.
        using Cursor = RecordCollection::LinearRepresentation::DescendingReadCursor;

        Cursor Execute (
            RecordCollection::LinearRepresentation::KeyFieldValue _min,
            RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept;

        StandardLayout::Field GetKeyField () const noexcept;

    private:
        friend class LongTermContainer;

        FetchReversedRangeQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::LinearRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       given min max interval contains key field value.
    class ModifyReversedRangeQuery final : public RepresentationQueryBase <RecordCollection::LinearRepresentation>
    {
    public:
        /// \details Objects are sorted in descending order on key field value.
        using Cursor = RecordCollection::LinearRepresentation::DescendingEditCursor;

        Cursor Execute (
            RecordCollection::LinearRepresentation::KeyFieldValue _min,
            RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept;

        StandardLayout::Field GetKeyField () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyReversedRangeQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::LinearRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given shape.
    class FetchShapeIntersectionQuery final :
        public RepresentationQueryBase <RecordCollection::VolumetricRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::VolumetricRepresentation::ShapeIntersectionReadCursor;

        Cursor Execute (RecordCollection::VolumetricRepresentation::Shape _shape) noexcept;

        RecordCollection::VolumetricRepresentation::DimensionIterator DimensionBegin () const noexcept;

        RecordCollection::VolumetricRepresentation::DimensionIterator DimensionEnd () const noexcept;

    private:
        friend class LongTermContainer;

        FetchShapeIntersectionQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::VolumetricRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given shape.
    class ModifyShapeIntersectionQuery final :
        public RepresentationQueryBase <RecordCollection::VolumetricRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::VolumetricRepresentation::ShapeIntersectionEditCursor;

        Cursor Execute (RecordCollection::VolumetricRepresentation::Shape _shape) noexcept;

        RecordCollection::VolumetricRepresentation::DimensionIterator DimensionBegin () const noexcept;

        RecordCollection::VolumetricRepresentation::DimensionIterator DimensionEnd () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyShapeIntersectionQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::VolumetricRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given ray and distance
    ///        from intersection point to ray origin is less or equal to given max distance.
    class FetchRayIntersectionQuery final :
        public RepresentationQueryBase <RecordCollection::VolumetricRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::VolumetricRepresentation::RayIntersectionReadCursor;

        Cursor Execute (RecordCollection::VolumetricRepresentation::Ray _ray, float _maxDistance) noexcept;

        RecordCollection::VolumetricRepresentation::DimensionIterator DimensionBegin () const noexcept;

        RecordCollection::VolumetricRepresentation::DimensionIterator DimensionEnd () const noexcept;

    private:
        friend class LongTermContainer;

        FetchRayIntersectionQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::VolumetricRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given ray and distance
    ///        from intersection point to ray origin is less or equal to given max distance.
    class ModifyRayIntersectionQuery final :
        public RepresentationQueryBase <RecordCollection::VolumetricRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::VolumetricRepresentation::RayIntersectionEditCursor;

        Cursor Execute (RecordCollection::VolumetricRepresentation::Ray _ray, float _maxDistance) noexcept;

        RecordCollection::VolumetricRepresentation::DimensionIterator DimensionBegin () const noexcept;

        RecordCollection::VolumetricRepresentation::DimensionIterator DimensionEnd () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyRayIntersectionQuery (
            Handling::Handle <LongTermContainer> _container,
            RecordCollection::VolumetricRepresentation _representation) noexcept;
    };

    InsertQuery Insert () noexcept;

    FetchValueQuery FetchValue (const std::vector <StandardLayout::FieldId> &_keyFields) noexcept;

    ModifyValueQuery ModifyValue (const std::vector <StandardLayout::FieldId> &_keyFields) noexcept;

    FetchRangeQuery FetchRange (StandardLayout::FieldId _keyField) noexcept;

    ModifyRangeQuery ModifyRange (StandardLayout::FieldId _keyField) noexcept;

    FetchReversedRangeQuery FetchReversedRange (StandardLayout::FieldId _keyField) noexcept;

    ModifyReversedRangeQuery ModifyReversedRange (StandardLayout::FieldId _keyField) noexcept;

    FetchShapeIntersectionQuery FetchShapeIntersection (
        const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

    ModifyShapeIntersectionQuery ModifyShapeIntersection (
        const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

    FetchRayIntersectionQuery FetchRayIntersection (
        const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

    ModifyRayIntersectionQuery ModifyRayIntersection (
        const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

private:
    /// CargoDeck constructs containers.
    friend class CargoDeck;

    /// Only handles have right to destruct containers.
    template <typename>
    friend class Handling::Handle;

    explicit LongTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept;

    ~LongTermContainer () noexcept;

    RecordCollection::LinearRepresentation AcquireLinearRepresentation (StandardLayout::FieldId _keyField) noexcept;

    // TODO: Value reordering for value queries is not supported. Therefore preparing query for fields A B C and
    //       C B A will result on creation of two separate representations. Think about fixing this problem.
    //       There is same problem with volumetric query dimension reordering.

    RecordCollection::PointRepresentation AcquirePointRepresentation (
        const std::vector <StandardLayout::FieldId> &_keyFields) noexcept;

    RecordCollection::VolumetricRepresentation AcquireVolumetricRepresentation (
        const std::vector <RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

    RecordCollection::Collection collection;
};

template <typename Representation>
LongTermContainer::RepresentationQueryBase <Representation>::~RepresentationQueryBase () noexcept
{
    // If prepared query was moved out, representation call will result in undefined behaviour.
    // Therefore, we should check container reference first. It will be null of query was moved out.
    if (container && representation.CanBeDropped ())
    {
        representation.Drop ();
    }
}

template <typename Representation>
Handling::Handle <LongTermContainer>
LongTermContainer::RepresentationQueryBase <Representation>::GetContainer () const noexcept
{
    assert (container);
    return container;
}

template <typename Representation>
LongTermContainer::RepresentationQueryBase <Representation>::RepresentationQueryBase (
    Handling::Handle <LongTermContainer> _container, Representation _representation) noexcept
    : container (std::move (_container)),
      representation (std::move (_representation))
{
    assert (container);
}
} // namespace Emergence::Galleon
