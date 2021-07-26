#pragma once

#include <array>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Warehouse
{
/// \brief Storage for objects that are created and destroyed frequently.
/// \details ShortTermStorage object is shared-ownership handle for implementation instance. Storage will be
///          automatically destroyed if there are no handles for this storage or its prepared queries.
class ShortTermStorage final
{
public:
    /// \brief Prepared query, used to start insertion transactions.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there are no handles for it.
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
            Cursor &operator = (const Cursor &_other) = delete;

            /// Assigning cursors looks counter intuitive.
            Cursor &operator = (Cursor &&_other) = delete;

        private:
            /// Query constructs its cursors.
            friend class InsertQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            /// \brief Implementation-specific data.
            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        InsertQuery (const InsertQuery &_other) noexcept;

        InsertQuery (InsertQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~InsertQuery ();

        /// \return Cursor, that allows user to insert new objects into storage.
        /// \invariant There is no other cursors in this storage.
        Cursor Insert () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        InsertQuery &operator = (const InsertQuery &_other) = delete;

        /// Assigning prepared queries looks counter intuitive.
        InsertQuery &operator = (InsertQuery &&_other) = delete;

    private:
        /// Storage constructs prepared queries.
        friend class ShortTermStorage;

        explicit InsertQuery (void *_handle) noexcept;

        /// \brief Implementation handle.
        void *handle;
    };

    /// \brief Prepared query, used to gain readonly access to stored objects.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there are no handles for it.
    class FetchQuery final
    {
    public:
        /// \brief Provides thread safe readonly access to stored objects.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) noexcept;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            /// \return Pointer to current object or nullptr if cursor points to ending.
            const void *operator * () const noexcept;

            /// \brief Moves cursor to next object.
            /// \invariant Cursor should not point to ending.
            Cursor &operator ++ () noexcept;

            /// Assigning cursors looks counter intuitive.
            Cursor &operator = (const Cursor &_other) = delete;

            /// Assigning cursors looks counter intuitive.
            Cursor &operator = (Cursor &&_other) = delete;

        private:
            /// Query constructs its cursors.
            friend class FetchQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            /// \brief Implementation-specific data.
            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        FetchQuery (const FetchQuery &_other) noexcept;

        FetchQuery (FetchQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~FetchQuery ();

        /// \return Cursor, that provides thread safe read only access to stored objects.
        /// \details Thread safe.
        /// \invariant There is no insertion or modification cursors in this storage.
        Cursor Execute () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        FetchQuery &operator = (const FetchQuery &_other) = delete;

        /// Assigning prepared queries looks counter intuitive.
        FetchQuery &operator = (FetchQuery &&_other) = delete;

    private:
        /// Storage constructs prepared queries.
        friend class ShortTermStorage;

        explicit FetchQuery (void *_handle) noexcept;

        /// \brief Implementation handle.
        void *handle;
    };

    /// \brief Prepared query, used to gain readwrite access to stored objects.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there are no handles for it.
    class ModifyQuery final
    {
    public:
        /// \brief Provides readwrite access to stored objects.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            /// \return Pointer to current object or nullptr if cursor points to ending.
            void *operator * () const noexcept;

            /// \brief Deletes current object from collection and moves to next record.
            /// \invariant Cursor should not point to ending.
            /// \warning Object type is unknown during compile time, therefore appropriate
            ///          destructor should be called before record deletion.
            void operator ~ () noexcept;

            /// \brief Moves cursor to next object.
            /// \invariant Cursor should not point to ending.
            Cursor &operator ++ () noexcept;

            /// Assigning cursors looks counter intuitive.
            Cursor &operator = (const Cursor &_other) = delete;

            /// Assigning cursors looks counter intuitive.
            Cursor &operator = (Cursor &&_other) = delete;

        private:
            /// Query constructs its cursors.
            friend class ModifyQuery;

            static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

            /// \brief Implementation-specific data.
            std::array <uint8_t, DATA_MAX_SIZE> data;
        };

        ModifyQuery (const ModifyQuery &_other) noexcept;

        ModifyQuery (ModifyQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~ModifyQuery ();

        /// \return Cursor, that provides readwrite access to stored objects.
        /// \invariant There is no other cursors in this storage.
        Cursor Execute () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        ModifyQuery &operator = (const ModifyQuery &_other) = delete;

        /// Assigning prepared queries looks counter intuitive.
        ModifyQuery &operator = (ModifyQuery &&_other) = delete;

    private:
        /// Storage constructs prepared queries.
        friend class ShortTermStorage;

        explicit ModifyQuery (void *_handle) noexcept;

        /// \brief Implementation handle.
        void *handle;
    };

    ShortTermStorage (const ShortTermStorage &_other) noexcept;

    ShortTermStorage (ShortTermStorage &&_other) noexcept;

    ~ShortTermStorage () noexcept;

    /// \return Mapping for objects type.
    StandardLayout::Mapping GetTypeMapping () const noexcept;

    /// \return Prepared query for object insertion.
    InsertQuery Insert () noexcept;

    /// \return Prepared query for read only access.
    FetchQuery Fetch () noexcept;

    /// \return Prepared query for readwrite access.
    ModifyQuery Modify () noexcept;

    /// Assigning storage handles looks counter intuitive.
    ShortTermStorage &operator = (const ShortTermStorage &_other) = delete;

    /// Assigning storage handles looks counter intuitive.
    ShortTermStorage &operator = (ShortTermStorage &&_other) = delete;

private:
    /// Registry constructs storages.
    friend class Registry;

    explicit ShortTermStorage (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::Warehouse