#pragma once

#include <array>
#include <vector>

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Warehouse
{
/// \brief Storage for objects that are created and destroyed rarely.
/// \details Prepared queries for this storage are supported by indexation to provide fast parametrized lookups.
///          LongTermStorage object is shared-ownership handle for implementation instance. Storage will be
///          automatically destroyed if there is no handles for this storage or its prepared queries.
class LongTermStorage final
{
public:
    /// \brief Defines sequence of values by specifying value for each key field of FetchValue or ModifyValueQuery.
    ///
    /// \details Key field count and types are unknown during compile time, therefore ValueSequence is
    ///          a pointer to memory block, that holds values for each key field in correct order.
    /// \see ::FetchValue
    /// \see ::ModifyValue
    ///
    /// \warning Due to runtime-only nature of sequences, logically incorrect pointers can not be caught.
    /// \invariant Should not be `nullptr`.
    /// \invariant Values must be stored one after another without paddings in the same order as key fields.
    /// \invariant Value size for fields with StandardLayout::FieldArchetype::STRING must always be equal to
    ///            StandardLayout::Field::GetSize, even if string length is less than this value.
    /// \invariant Values for fields with StandardLayout::FieldArchetype::BIT must passed as bytes in which all
    ///            bits should be zero's except bit with StandardLayout::Field::GetBitOffset.
    using ValueSequence = void *;

    /// \brief Points to value, that defines one of interval bounds for range lookup.
    ///
    /// \details Key field type is unknown during compile time, therefore value is a pointer to memory
    ///          block with actual value. `nullptr` values will be interpreted as absence of borders.
    /// \see ::FetchRange
    /// \see ::ModifyRange
    /// \see ::FetchReversedRange
    /// \see ::ModifyReversedRange
    ///
    /// \warning Due to runtime-only nature of values, logically incorrect pointers can not be caught.
    using Bound = void *;

    /// \brief Defines shape by specifying min-max value pair for each dimension.
    ///
    /// \details Dimension count and types are unknown during compile time, therefore Shape is a pointer to
    ///          memory block, that holds min-max pair of values for each dimension in correct order. For example,
    ///          if it's needed to describe rectangle with width equal to 3, height equal to 2, center in
    ///          (x = 1, y = 3} point and dimensions are x = {float x0; float x1;} and y = {float y0; float y1;},
    ///          then shape memory block should be {-0.5f, 2.5f, 2.0f, 4.0f}.
    ///
    /// \see ::FetchShapeIntersections
    /// \see ::ModifyShapeIntersections
    ///
    /// \warning Due to runtime-only nature of shapes, logically incorrect pointers can not be caught.
    /// \invariant Should not be `nullptr`.
    using Shape = void *;

    /// \brief Defines ray by specifying origin-direction value pair for each dimension.
    ///
    /// \details Dimension count and types are unknown during compile time, therefore Ray is a pointer to
    ///          memory block, that holds origin-direction pair of values for each dimension in correct order.
    ///          For example, if it's needed to describe ray with origin in (x = 2, y = 3) point,
    ///          (dx = 0.8, dy = -0.6) direction and dimensions are x = {float x0; float x1;} and
    ///          y = {float y0; float y1;}, then ray memory block should be {2.0f, 0.8f, 3.0f, -0.6f}.
    ///
    /// \see ::FetchRayIntersections
    /// \see ::ModifyRayIntersections
    ///
    /// \warning Due to runtime-only nature of rays, logically incorrect pointers can not be caught.
    /// \invariant Should not be `nullptr`.
    using Ray = void *;

    /// \brief Prepared query, used to start insertion transactions.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class InsertQuery final
    {
    public:
        /// \brief Allows user to allocate new objects and insert them into this storage.
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept;

            /// \invariant Previously allocated object must be initialized before cursor destruction.
            ~Cursor () noexcept;

            /// \return Pointer to memory, allocated for the new object.
            /// \invariant Previously allocated object must be initialized before next call.
            void *operator ++ () noexcept;

            /// Assigning cursors looks counter intuitive.
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);

        private:
            /// Query constructs its cursors.
            friend class InsertQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        InsertQuery (const InsertQuery &_other) noexcept;

        InsertQuery (InsertQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~InsertQuery ();

        /// \return Cursor, that allows user to insert new objects into storage.
        /// \invariant There is no other cursors in this storage.
        Cursor Insert () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (InsertQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit InsertQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ///        each key field value is equal to according value in given values sequence.
    /// \details Key fields are selected during prepared query creation using ::FetchValue.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class FetchValueQuery final
    {
    public:
        /// \brief Provides thread safe readonly access objects that match criteria in FetchValueQuery brief.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class FetchValueQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        FetchValueQuery (const FetchValueQuery &_other) noexcept;

        FetchValueQuery (FetchValueQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~FetchValueQuery ();

        /// \return Cursor, that provides thread safe readonly access to objects
        ///         that match criteria in FetchValueQuery brief.
        /// \details Thread safe.
        /// \invariant There is no insertion or modification cursors in this storage.
        Cursor Execute (const ValueSequence _values) noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchValueQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit FetchValueQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ///        each key field value is equal to according value in given values sequence.
    /// \details Key fields are selected during prepared query creation using ::ModifyValue.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class ModifyValueQuery final
    {
    public:
        /// \brief Provides readwrite access to objects that match criteria in ModifyValueQuery brief.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class ModifyValueQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        ModifyValueQuery (const ModifyValueQuery &_other) noexcept;

        ModifyValueQuery (ModifyValueQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~ModifyValueQuery ();

        /// \return Cursor, that provides readwrite access to objects that match criteria in ModifyValueQuery brief.
        /// \invariant There is no other cursors in this storage.
        Cursor Execute (const ValueSequence _values) noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyValueQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit ModifyValueQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ///        given min max interval contains key field value.
    /// \details Key fields are selected during prepared query creation using ::FetchRange.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class FetchRangeQuery final
    {
    public:
        /// \brief Provides thread safe readonly access objects that match criteria in FetchRangeQuery brief.
        /// \details Objects are sorted in ascending order on key field value.
        class Cursor final
        {
        public:
            EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class FetchRangeQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        FetchRangeQuery (const FetchRangeQuery &_other) noexcept;

        FetchRangeQuery (FetchRangeQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~FetchRangeQuery ();

        /// \return Cursor, that provides thread safe readonly access to objects
        ///         that match criteria in FetchRangeQuery brief.
        /// \details Thread safe.
        /// \invariant There is no insertion or modification cursors in this storage.
        Cursor Execute (const Bound _min, const Bound _max) noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchRangeQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit FetchRangeQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       given min max interval contains key field value.
    /// \details Key fields are selected during prepared query creation using ::ModifyRange.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class ModifyRangeQuery final
    {
    public:
        /// \brief Provides readwrite access to objects that match criteria in ModifyRangeQuery brief.
        /// \details Objects are sorted in ascending order on key field value.
        class Cursor final
        {
        public:
            EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class ModifyRangeQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        ModifyRangeQuery (const ModifyRangeQuery &_other) noexcept;

        ModifyRangeQuery (ModifyRangeQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~ModifyRangeQuery ();

        /// \return Cursor, that provides readwrite access to objects that match criteria in ModifyRangeQuery brief.
        /// \invariant There is no other cursors in this storage.
        Cursor Execute (const Bound _min, const Bound _max) noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyRangeQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit ModifyRangeQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ///        given min max interval contains key field value.
    /// \details Key fields are selected during prepared query creation using ::FetchReversedRange.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class FetchReversedRangeQuery final
    {
    public:
        /// \brief Provides thread safe readonly access objects that match criteria in FetchReversedRangeQuery brief.
        /// \details Objects are sorted in descending order on key field value.
        class Cursor final
        {
        public:
            EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class FetchReversedRangeQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        FetchReversedRangeQuery (const FetchReversedRangeQuery &_other) noexcept;

        FetchReversedRangeQuery (FetchReversedRangeQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~FetchReversedRangeQuery ();

        /// \return Cursor, that provides thread safe readonly access to objects
        ///         that match criteria in FetchReversedRangeQuery brief.
        /// \details Thread safe.
        /// \invariant There is no insertion or modification cursors in this storage.
        Cursor Execute (const Bound _min, const Bound _max) noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchReversedRangeQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit FetchReversedRangeQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       given min max interval contains key field value.
    /// \details Key fields are selected during prepared query creation using ::ModifyReversedRange.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class ModifyReversedRangeQuery final
    {
    public:
        /// \brief Provides readwrite access to objects that match criteria in ModifyReversedRangeQuery brief.
        /// \details Objects are sorted in descending order on key field value.
        class Cursor final
        {
        public:
            EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class ModifyReversedRangeQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        ModifyReversedRangeQuery (const ModifyReversedRangeQuery &_other) noexcept;

        ModifyReversedRangeQuery (ModifyReversedRangeQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~ModifyReversedRangeQuery ();

        /// \return Cursor, that provides readwrite access to objects
        ///         that match criteria in ModifyReversedRangeQuery brief.
        /// \invariant There is no other cursors in this storage.
        Cursor Execute (const Bound _min, const Bound _max) noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyReversedRangeQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit ModifyReversedRangeQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given shape.
    /// \details Key fields are selected during prepared query creation using ::FetchShapeIntersections.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class FetchShapeIntersectionsQuery final
    {
    public:
        /// \brief Provides thread safe readonly access objects that
        ///        match criteria in FetchShapeIntersectionsQuery brief.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class FetchShapeIntersectionsQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        FetchShapeIntersectionsQuery (const FetchShapeIntersectionsQuery &_other) noexcept;

        FetchShapeIntersectionsQuery (FetchShapeIntersectionsQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~FetchShapeIntersectionsQuery ();

        /// \return Cursor, that provides thread safe readonly access to objects
        ///         that match criteria in FetchShapeIntersectionsQuery brief.
        /// \details Thread safe.
        /// \invariant There is no insertion or modification cursors in this storage.
        Cursor Execute (const Shape _shape) noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchShapeIntersectionsQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit FetchShapeIntersectionsQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given shape.
    /// \details Key fields are selected during prepared query creation using ::ModifyShapeIntersections.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class ModifyShapeIntersectionsQuery final
    {
    public:
        /// \brief Provides readwrite access to objects that match criteria in ModifyShapeIntersectionsQuery brief.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class ModifyShapeIntersectionsQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        ModifyShapeIntersectionsQuery (const ModifyShapeIntersectionsQuery &_other) noexcept;

        ModifyShapeIntersectionsQuery (ModifyShapeIntersectionsQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~ModifyShapeIntersectionsQuery ();

        /// \return Cursor, that provides readwrite access to objects
        ///         that match criteria in ModifyShapeIntersectionsQuery brief.
        /// \invariant There is no other cursors in this storage.
        Cursor Execute (const Shape _shape) noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyShapeIntersectionsQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit ModifyShapeIntersectionsQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain thread safe readonly access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given ray.
    /// \details Key fields are selected during prepared query creation using ::FetchRayIntersections.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class FetchRayIntersectionsQuery final
    {
    public:
        /// \brief Provides thread safe readonly access objects that
        ///        match criteria in FetchRayIntersectionsQuery brief.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class FetchRayIntersectionsQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        FetchRayIntersectionsQuery (const FetchRayIntersectionsQuery &_other) noexcept;

        FetchRayIntersectionsQuery (FetchRayIntersectionsQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~FetchRayIntersectionsQuery ();

        /// \return Cursor, that provides thread safe readonly access to objects
        ///         that match criteria in FetchRayIntersectionsQuery brief.
        /// \details Thread safe.
        /// \invariant There is no insertion or modification cursors in this storage.
        Cursor Execute (const Ray _ray) noexcept;

        // TODO: Fetch/Modify closes intersection query?

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchRayIntersectionsQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit FetchRayIntersectionsQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain readwrite access to objects that match criteria:
    ////       shape, described by values of object key dimensions, intersects with given ray.
    /// \details Key fields are selected during prepared query creation using ::ModifyRayIntersections.
    ///          Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class ModifyRayIntersectionsQuery final
    {
    public:
        /// \brief Provides readwrite access to objects that match criteria in ModifyRayIntersectionsQuery brief.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class ModifyRayIntersectionsQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        ModifyRayIntersectionsQuery (const ModifyRayIntersectionsQuery &_other) noexcept;

        ModifyRayIntersectionsQuery (ModifyRayIntersectionsQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~ModifyRayIntersectionsQuery ();

        /// \return Cursor, that provides readwrite access to objects
        ///         that match criteria in ModifyRayIntersectionsQuery brief.
        /// \invariant There is no other cursors in this storage.
        Cursor Execute (const Ray _ray) noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyRayIntersectionsQuery);

    private:
        /// Storage constructs prepared queries.
        friend class LongTermStorage;

        explicit ModifyRayIntersectionsQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Describes one dimensions for volumetric prepared queries.
    struct DimensionDescriptor
    {
        /// \brief Pointer to minimum possible value of #minBorderField.
        ///
        /// \details Values, that are less than this value will be processed as this value.
        ///          Guarantied to be copied during prepared query creation, therefore can point to stack memory.
        const void *globalMinBorder;

        /// \brief Id of field, that holds record minimum border value for this dimension.
        ///
        /// \invariant Field archetype is FieldArchetype::INT, FieldArchetype::UINT or FieldArchetype::FLOAT.
        StandardLayout::FieldId minBorderField;

        /// \brief Pointer to maximum possible value of #maxBorderField.
        ///
        /// \details Values, that are greater than this value will be processed as this value.
        ///          Guarantied to be copied during prepared query creation, therefore can point to stack memory.
        const void *globalMaxBorder;

        /// \brief Id of field, that holds record maximum border value for this dimension.
        ///
        /// \invariant Field archetype is FieldArchetype::INT, FieldArchetype::UINT or FieldArchetype::FLOAT.
        StandardLayout::FieldId maxBorderField;
    };

    LongTermStorage (const LongTermStorage &_other) noexcept;

    LongTermStorage (LongTermStorage &&_other) noexcept;

    ~LongTermStorage () noexcept;

    /// \return Mapping for objects type.
    StandardLayout::Mapping GetTypeMapping () const noexcept;

    /// \return Prepared query for object insertion.
    InsertQuery Insert () noexcept;

    /// \return Readonly select-value prepared query on given key fields.
    FetchValueQuery FetchValue (const std::vector <StandardLayout::FieldId> &_keyFields) noexcept;

    /// \return Readwrite select-value prepared query on given key fields.
    ModifyValueQuery ModifyValue (const std::vector <StandardLayout::FieldId> &_keyFields) noexcept;

    /// \return Readonly select-range prepared query with ascending order on given key field.
    FetchRangeQuery FetchRange (StandardLayout::FieldId _keyField) noexcept;

    /// \return Readwrite select-range prepared query with ascending order on given key field.
    ModifyRangeQuery ModifyRange (StandardLayout::FieldId _keyField) noexcept;

    /// \return Readonly select-range prepared query with descending order on given key field.
    FetchReversedRangeQuery FetchReversedRange (StandardLayout::FieldId _keyField) noexcept;

    /// \return Readwrite select-range prepared query with descending order on given key field.
    ModifyReversedRangeQuery ModifyReversedRange (StandardLayout::FieldId _keyField) noexcept;

    /// \return Readonly select-shape-intersections prepared query on given dimensions.
    FetchShapeIntersectionsQuery FetchShapeIntersections (
        const std::vector <DimensionDescriptor> &_dimensions) noexcept;

    /// \return Readwrite select-shape-intersections prepared query on given dimensions.
    ModifyShapeIntersectionsQuery ModifyShapeIntersections (
        const std::vector <DimensionDescriptor> &_dimensions) noexcept;

    /// \return Readonly select-ray-intersections prepared query on given dimensions.
    FetchRayIntersectionsQuery FetchRayIntersections (const std::vector <DimensionDescriptor> &_dimensions) noexcept;

    /// \return Readwrite select-ray-intersections prepared query on given dimensions.
    ModifyRayIntersectionsQuery ModifyRayIntersections (const std::vector <DimensionDescriptor> &_dimensions) noexcept;

    /// Assigning storage handles looks counter intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (LongTermStorage);

private:
    /// Registry constructs storages.
    friend class Registry;

    explicit LongTermStorage (void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Warehouse