#pragma once

#include <API/Common/MuteWarnings.hpp>

#include <API/Common/Cursor.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Assert/Assert.hpp>

#include <Container/TypedOrderedPool.hpp>

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

            ~Cursor () noexcept = default;

            void *operator++ () noexcept;

            /// Assigning cursors looks counter intuitive.
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);

        private:
            friend class InsertQuery;

            explicit Cursor (Handling::Handle<LongTermContainer> _container) noexcept;

            Handling::Handle<LongTermContainer> container;
            RecordCollection::Collection::Allocator allocator;
        };

        InsertQuery (const InsertQuery &_other) noexcept = default;

        InsertQuery (InsertQuery &&_other) noexcept = default;

        ~InsertQuery () noexcept = default;

        [[nodiscard]] Cursor Execute () const noexcept;

        [[nodiscard]] Handling::Handle<LongTermContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (InsertQuery);

    private:
        friend class LongTermContainer;

        explicit InsertQuery (Handling::Handle<LongTermContainer> _container) noexcept;

        Handling::Handle<LongTermContainer> container;
    };

    template <typename Representation>
    class RepresentationQueryBase
    {
    public:
        RepresentationQueryBase (const RepresentationQueryBase &_other) noexcept = default;

        RepresentationQueryBase (RepresentationQueryBase &&_other) noexcept = default;

        ~RepresentationQueryBase () noexcept;

        [[nodiscard]] Handling::Handle<LongTermContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (RepresentationQueryBase);

    protected:
        /// VisualizationDriver for Warehouse service should be able to directly access ::representation.
        friend class VisualizationDriver;

        RepresentationQueryBase (Handling::Handle<LongTermContainer> _container,
                                 Representation _representation) noexcept;

        Handling::Handle<LongTermContainer> container;
        Representation representation;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ///        each key field value is equal to according value in given values sequence.
    class FetchValueQuery final : public RepresentationQueryBase<RecordCollection::PointRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::PointRepresentation::ReadCursor;

        Cursor Execute (RecordCollection::PointRepresentation::Point _values) noexcept;

        [[nodiscard]] RecordCollection::PointRepresentation::KeyFieldIterator KeyFieldBegin () const noexcept;

        [[nodiscard]] RecordCollection::PointRepresentation::KeyFieldIterator KeyFieldEnd () const noexcept;

    private:
        friend class LongTermContainer;

        FetchValueQuery (Handling::Handle<LongTermContainer> _container,
                         RecordCollection::PointRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ///        each key field value is equal to according value in given values sequence.
    class ModifyValueQuery final : public RepresentationQueryBase<RecordCollection::PointRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::PointRepresentation::EditCursor;

        Cursor Execute (RecordCollection::PointRepresentation::Point _values) noexcept;

        [[nodiscard]] RecordCollection::PointRepresentation::KeyFieldIterator KeyFieldBegin () const noexcept;

        [[nodiscard]] RecordCollection::PointRepresentation::KeyFieldIterator KeyFieldEnd () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyValueQuery (Handling::Handle<LongTermContainer> _container,
                          RecordCollection::PointRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ///        given min max interval contains key field value.
    class FetchAscendingRangeQuery final : public RepresentationQueryBase<RecordCollection::LinearRepresentation>
    {
    public:
        using Cursor = RecordCollection::LinearRepresentation::AscendingReadCursor;

        Cursor Execute (RecordCollection::LinearRepresentation::KeyFieldValue _min,
                        RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept;

        [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

    private:
        friend class LongTermContainer;

        FetchAscendingRangeQuery (Handling::Handle<LongTermContainer> _container,
                                  RecordCollection::LinearRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       given min max interval contains key field value.
    class ModifyAscendingRangeQuery final : public RepresentationQueryBase<RecordCollection::LinearRepresentation>
    {
    public:
        using Cursor = RecordCollection::LinearRepresentation::AscendingEditCursor;

        Cursor Execute (RecordCollection::LinearRepresentation::KeyFieldValue _min,
                        RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept;

        [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyAscendingRangeQuery (Handling::Handle<LongTermContainer> _container,
                                   RecordCollection::LinearRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ///        given min max interval contains key field value.
    class FetchDescendingRangeQuery final : public RepresentationQueryBase<RecordCollection::LinearRepresentation>
    {
    public:
        using Cursor = RecordCollection::LinearRepresentation::DescendingReadCursor;

        Cursor Execute (RecordCollection::LinearRepresentation::KeyFieldValue _min,
                        RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept;

        [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

    private:
        friend class LongTermContainer;

        FetchDescendingRangeQuery (Handling::Handle<LongTermContainer> _container,
                                   RecordCollection::LinearRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       given min max interval contains key field value.
    class ModifyDescendingRangeQuery final : public RepresentationQueryBase<RecordCollection::LinearRepresentation>
    {
    public:
        /// \details Objects are sorted in descending order on key field value.
        using Cursor = RecordCollection::LinearRepresentation::DescendingEditCursor;

        Cursor Execute (RecordCollection::LinearRepresentation::KeyFieldValue _min,
                        RecordCollection::LinearRepresentation::KeyFieldValue _max) noexcept;

        [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyDescendingRangeQuery (Handling::Handle<LongTermContainer> _container,
                                    RecordCollection::LinearRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that are considered signaled based on
    ///        field value, selected during query creation.
    class FetchSignalQuery final : public RepresentationQueryBase<RecordCollection::SignalRepresentation>
    {
    public:
        using Cursor = RecordCollection::SignalRepresentation::ReadCursor;

        Cursor Execute () noexcept;

        [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

        [[nodiscard]] bool IsSignaledValue (const std::array<std::uint8_t, sizeof (std::uint64_t)> &_value) const;

        [[nodiscard]] std::array<std::uint8_t, sizeof (std::uint64_t)> GetSignaledValue () const noexcept;

    private:
        friend class LongTermContainer;

        FetchSignalQuery (Handling::Handle<LongTermContainer> _container,
                          RecordCollection::SignalRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that are considered signaled based on
    //         field value, selected during query creation.
    class ModifySignalQuery final : public RepresentationQueryBase<RecordCollection::SignalRepresentation>
    {
    public:
        using Cursor = RecordCollection::SignalRepresentation::EditCursor;

        Cursor Execute () noexcept;

        [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

        [[nodiscard]] bool IsSignaledValue (const std::array<std::uint8_t, sizeof (std::uint64_t)> &_value) const;

        [[nodiscard]] std::array<std::uint8_t, sizeof (std::uint64_t)> GetSignaledValue () const noexcept;

    private:
        friend class LongTermContainer;

        ModifySignalQuery (Handling::Handle<LongTermContainer> _container,
                           RecordCollection::SignalRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given shape.
    class FetchShapeIntersectionQuery final : public RepresentationQueryBase<RecordCollection::VolumetricRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::VolumetricRepresentation::ShapeIntersectionReadCursor;

        Cursor Execute (RecordCollection::VolumetricRepresentation::Shape _shape) noexcept;

        [[nodiscard]] RecordCollection::VolumetricRepresentation::DimensionIterator DimensionBegin () const noexcept;

        [[nodiscard]] RecordCollection::VolumetricRepresentation::DimensionIterator DimensionEnd () const noexcept;

    private:
        friend class LongTermContainer;

        FetchShapeIntersectionQuery (Handling::Handle<LongTermContainer> _container,
                                     RecordCollection::VolumetricRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given shape.
    class ModifyShapeIntersectionQuery final
        : public RepresentationQueryBase<RecordCollection::VolumetricRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::VolumetricRepresentation::ShapeIntersectionEditCursor;

        Cursor Execute (RecordCollection::VolumetricRepresentation::Shape _shape) noexcept;

        [[nodiscard]] RecordCollection::VolumetricRepresentation::DimensionIterator DimensionBegin () const noexcept;

        [[nodiscard]] RecordCollection::VolumetricRepresentation::DimensionIterator DimensionEnd () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyShapeIntersectionQuery (Handling::Handle<LongTermContainer> _container,
                                      RecordCollection::VolumetricRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given ray and distance
    ///        from intersection point to ray origin is less or equal to given max distance.
    class FetchRayIntersectionQuery final : public RepresentationQueryBase<RecordCollection::VolumetricRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::VolumetricRepresentation::RayIntersectionReadCursor;

        Cursor Execute (RecordCollection::VolumetricRepresentation::Ray _ray, float _maxDistance) noexcept;

        [[nodiscard]] RecordCollection::VolumetricRepresentation::DimensionIterator DimensionBegin () const noexcept;

        [[nodiscard]] RecordCollection::VolumetricRepresentation::DimensionIterator DimensionEnd () const noexcept;

    private:
        friend class LongTermContainer;

        FetchRayIntersectionQuery (Handling::Handle<LongTermContainer> _container,
                                   RecordCollection::VolumetricRepresentation _representation) noexcept;
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given ray and distance
    ///        from intersection point to ray origin is less or equal to given max distance.
    class ModifyRayIntersectionQuery final : public RepresentationQueryBase<RecordCollection::VolumetricRepresentation>
    {
    public:
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        using Cursor = RecordCollection::VolumetricRepresentation::RayIntersectionEditCursor;

        Cursor Execute (RecordCollection::VolumetricRepresentation::Ray _ray, float _maxDistance) noexcept;

        [[nodiscard]] RecordCollection::VolumetricRepresentation::DimensionIterator DimensionBegin () const noexcept;

        [[nodiscard]] RecordCollection::VolumetricRepresentation::DimensionIterator DimensionEnd () const noexcept;

    private:
        friend class LongTermContainer;

        ModifyRayIntersectionQuery (Handling::Handle<LongTermContainer> _container,
                                    RecordCollection::VolumetricRepresentation _representation) noexcept;
    };

    LongTermContainer (const LongTermContainer &_other) = delete;

    LongTermContainer (LongTermContainer &&_other) = delete;

    InsertQuery Insert () noexcept;

    FetchValueQuery FetchValue (const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    ModifyValueQuery ModifyValue (const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    FetchAscendingRangeQuery FetchAscendingRange (StandardLayout::FieldId _keyField) noexcept;

    ModifyAscendingRangeQuery ModifyAscendingRange (StandardLayout::FieldId _keyField) noexcept;

    FetchDescendingRangeQuery FetchDescendingRange (StandardLayout::FieldId _keyField) noexcept;

    ModifyDescendingRangeQuery ModifyDescendingRange (StandardLayout::FieldId _keyField) noexcept;

    FetchSignalQuery FetchSignal (StandardLayout::FieldId _keyField,
                                  const std::array<std::uint8_t, sizeof (std::uint64_t)> &_signaledValue) noexcept;

    ModifySignalQuery ModifySignal (StandardLayout::FieldId _keyField,
                                    const std::array<std::uint8_t, sizeof (std::uint64_t)> &_signaledValue) noexcept;

    FetchShapeIntersectionQuery FetchShapeIntersection (
        const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

    ModifyShapeIntersectionQuery ModifyShapeIntersection (
        const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

    FetchRayIntersectionQuery FetchRayIntersection (
        const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

    ModifyRayIntersectionQuery ModifyRayIntersection (
        const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

    void LastReferenceUnregistered () noexcept;

    void SetUnsafeFetchAllowed (bool _allowed) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (LongTermContainer);

private:
    /// Pool from CargoDeck constructs and destructs containers.
    template <typename Item>
    friend class Container::TypedOrderedPool;

    /// VisualizationDriver for Warehouse service should be able to directly access ::collection.
    friend class VisualizationDriver;

    explicit LongTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept;

    ~LongTermContainer () noexcept = default;

    RecordCollection::LinearRepresentation AcquireLinearRepresentation (StandardLayout::FieldId _keyField) noexcept;

    // TODO: Value reordering for value queries is not supported. Therefore preparing query for fields A B C and
    //       C B A will result on creation of two separate representations. Think about fixing this problem.
    //       There is same problem with volumetric query dimension reordering.

    RecordCollection::PointRepresentation AcquirePointRepresentation (
        const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    RecordCollection::SignalRepresentation AcquireSignalRepresentation (
        StandardLayout::FieldId _keyField,
        const std::array<std::uint8_t, sizeof (std::uint64_t)> &_signaledValue) noexcept;

    RecordCollection::VolumetricRepresentation AcquireVolumetricRepresentation (
        const Container::Vector<RecordCollection::Collection::DimensionDescriptor> &_dimensions) noexcept;

    RecordCollection::Collection collection;
};

BEGIN_MUTING_OLD_DESTRUCTOR_NAME
template <typename Representation>
LongTermContainer::RepresentationQueryBase<Representation>::~RepresentationQueryBase () noexcept
{
    END_MUTING_WARNINGS
    // If prepared query was moved out, representation call will result in undefined behaviour.
    // Therefore, we should check container reference first. It will be null of query was moved out.
    if (container && representation.CanBeDropped ())
    {
        representation.Drop ();
    }
}

template <typename Representation>
Handling::Handle<LongTermContainer> LongTermContainer::RepresentationQueryBase<Representation>::GetContainer ()
    const noexcept
{
    EMERGENCE_ASSERT (container);
    return container;
}

template <typename Representation>
LongTermContainer::RepresentationQueryBase<Representation>::RepresentationQueryBase (
    Handling::Handle<LongTermContainer> _container, Representation _representation) noexcept
    : container (std::move (_container)),
      representation (std::move (_representation))
{
    EMERGENCE_ASSERT (container);
}
} // namespace Emergence::Galleon
