#pragma once

#include <array>
#include <vector>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Warehouse
{
class LongTermStorage final
{
public:
    using ValueSequence = void *;

    using Bound = void *;

    using Shape = void *;

    using Ray = void *;

    class InsertQuery final
    {
    public:
        class Inserter final
        {
        public:
            Inserter (const Inserter &_other) = delete;

            Inserter (Inserter &&_other) noexcept;

            ~Inserter () noexcept;

            void *Next () noexcept;

            Inserter &operator = (const Inserter &_other) = delete;

            Inserter &operator = (Inserter &&_other) = delete;
        };

        ~InsertQuery ();

        Inserter Insert () noexcept;

    private:
        friend class LongTermStorage;

        explicit InsertQuery (void *_handle) noexcept;

        void *handle;
    };

    class FetchByValueQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) noexcept;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            const void *operator * () const noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToReadQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~FetchByValueQuery ();

        Cursor Execute (const ValueSequence *_values) noexcept;

    private:
        friend class LongTermStorage;

        explicit FetchByValueQuery (void *_handle) noexcept;

        void *handle;
    };

    class ModifyByValueQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            void *operator * () const noexcept;

            void operator ~ () noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToEditQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~ModifyByValueQuery ();

        Cursor Execute (const ValueSequence *_values) noexcept;

    private:
        friend class LongTermStorage;

        explicit ModifyByValueQuery (void *_handle) noexcept;

        void *handle;
    };

    class FetchRangeQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) noexcept;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            const void *operator * () const noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToReadQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~FetchRangeQuery ();

        Cursor Execute (const Bound _min, const Bound _max) noexcept;

    private:
        friend class LongTermStorage;

        explicit FetchRangeQuery (void *_handle) noexcept;

        void *handle;
    };

    class ModifyRangeQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            void *operator * () const noexcept;

            void operator ~ () noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToEditQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~ModifyRangeQuery ();

        Cursor Execute (const Bound _min, const Bound _max) noexcept;

    private:
        friend class LongTermStorage;

        explicit ModifyRangeQuery (void *_handle) noexcept;

        void *handle;
    };

    class FetchReversedRangeQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) noexcept;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            const void *operator * () const noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToReadQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~FetchReversedRangeQuery ();

        Cursor Execute (const Bound _min, const Bound _max) noexcept;

    private:
        friend class LongTermStorage;

        explicit FetchReversedRangeQuery (void *_handle) noexcept;

        void *handle;
    };

    class ModifyReversedRangeQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            void *operator * () const noexcept;

            void operator ~ () noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToEditQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~ModifyReversedRangeQuery ();

        Cursor Execute (const Bound _min, const Bound _max) noexcept;

    private:
        friend class LongTermStorage;

        explicit ModifyReversedRangeQuery (void *_handle) noexcept;

        void *handle;
    };

    class FetchShapeIntersectionsQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) noexcept;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            const void *operator * () const noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToReadQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~FetchShapeIntersectionsQuery ();

        Cursor Execute (const Shape _shape) noexcept;

    private:
        friend class LongTermStorage;

        explicit FetchShapeIntersectionsQuery (void *_handle) noexcept;

        void *handle;
    };

    class ModifyShapeIntersectionsQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            void *operator * () const noexcept;

            void operator ~ () noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToEditQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~ModifyShapeIntersectionsQuery ();

        Cursor Execute (const Shape _shape) noexcept;

    private:
        friend class LongTermStorage;

        explicit ModifyShapeIntersectionsQuery (void *_handle) noexcept;

        void *handle;
    };

    class FetchRayIntersectionsQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) noexcept;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            const void *operator * () const noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToReadQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~FetchRayIntersectionsQuery ();

        Cursor Execute (const Ray _ray) noexcept;

    private:
        friend class LongTermStorage;

        explicit FetchRayIntersectionsQuery (void *_handle) noexcept;

        void *handle;
    };

    class ModifyRayIntersectionsQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            void *operator * () const noexcept;

            void operator ~ () noexcept;

            Cursor &operator ++ () noexcept;

            Cursor &operator = (const Cursor &_other) = delete;

            Cursor &operator = (Cursor &&_other) = delete;

        private:
            friend class FetchToEditQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ~ModifyRayIntersectionsQuery ();

        Cursor Execute (const Ray _ray) noexcept;

    private:
        friend class LongTermStorage;

        explicit ModifyRayIntersectionsQuery (void *_handle) noexcept;

        void *handle;
    };

    struct DimensionDescriptor
    {
        const void *globalMinBorder;

        StandardLayout::FieldId minBorderField;

        const void *globalMaxBorder;

        StandardLayout::FieldId maxBorderField;
    };

    ~LongTermStorage () noexcept;

    StandardLayout::Mapping GetTypeMapping () const noexcept;

    InsertQuery Insert () noexcept;

    FetchByValueQuery FetchByValue (const std::vector <StandardLayout::FieldId> &_keyFields) noexcept;

    ModifyByValueQuery ModifyByValue (const std::vector <StandardLayout::FieldId> &_keyFields) noexcept;

    FetchRangeQuery FetchRange (StandardLayout::FieldId _keyField) noexcept;

    ModifyRangeQuery ModifyRange (StandardLayout::FieldId _keyField) noexcept;

    FetchReversedRangeQuery FetchReversedRange (StandardLayout::FieldId _keyField) noexcept;

    ModifyReversedRangeQuery ModifyReversedRange (StandardLayout::FieldId _keyField) noexcept;

    FetchShapeIntersectionsQuery FetchShapeIntersections (
        const std::vector <DimensionDescriptor> &_dimensions) noexcept;

    ModifyShapeIntersectionsQuery ModifyShapeIntersections (
        const std::vector <DimensionDescriptor> &_dimensions) noexcept;

    FetchRayIntersectionsQuery FetchRayIntersections (const std::vector <DimensionDescriptor> &_dimensions) noexcept;

    ModifyRayIntersectionsQuery ModifyRayIntersections (const std::vector <DimensionDescriptor> &_dimensions) noexcept;

private:
    friend class Registry;

    explicit LongTermStorage (void *_handle) noexcept;

    void *handle;
};
} // namespace Emergence::Warehouse