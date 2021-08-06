#pragma once

#include <array>

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Warehouse
{
/// \brief Storage for objects that are created and destroyed frequently.
/// \details ShortTermStorage object is shared-ownership handle for implementation instance. Storage will be
///          automatically destroyed if there is no handles for this storage or its prepared queries.
class ShortTermStorage final
{
public:
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
        ~InsertQuery () noexcept;

        /// \return Cursor, that allows user to insert new objects into storage.
        /// \invariant There is no other cursors in this storage.
        Cursor Insert () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (InsertQuery);

    private:
        /// Storage constructs prepared queries.
        friend class ShortTermStorage;

        explicit InsertQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain thread safe readonly access to stored objects.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class FetchQuery final
    {
    public:
        /// \brief Provides thread safe readonly access to stored objects.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class FetchQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        FetchQuery (const FetchQuery &_other) noexcept;

        FetchQuery (FetchQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~FetchQuery () noexcept;

        /// \return Cursor, that provides thread safe readonly access to stored objects.
        /// \details Thread safe.
        /// \invariant There is no insertion or modification cursors in this storage.
        Cursor Execute () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchQuery);

    private:
        /// Storage constructs prepared queries.
        friend class ShortTermStorage;

        explicit FetchQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain readwrite access to stored objects.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class ModifyQuery final
    {
    public:
        /// \brief Provides readwrite access to stored objects.
        /// \warning There is no guaranteed order of objects. Therefore object order should be considered random.
        class Cursor final
        {
        public:
            EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

        private:
            /// Query constructs its cursors.
            friend class ModifyQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        ModifyQuery (const ModifyQuery &_other) noexcept;

        ModifyQuery (ModifyQuery &&_other) noexcept;

        /// \invariant There is no cursors for this query.
        ~ModifyQuery () noexcept;

        /// \return Cursor, that provides readwrite access to stored objects.
        /// \invariant There is no other cursors in this storage.
        Cursor Execute () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyQuery);

    private:
        /// Storage constructs prepared queries.
        friend class ShortTermStorage;

        explicit ModifyQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    ShortTermStorage (const ShortTermStorage &_other) noexcept;

    ShortTermStorage (ShortTermStorage &&_other) noexcept;

    ~ShortTermStorage () noexcept;

    /// \return Mapping for objects type.
    StandardLayout::Mapping GetTypeMapping () const noexcept;

    /// \return Prepared query for object insertion.
    InsertQuery Insert () noexcept;

    /// \return Prepared query for readonly access.
    FetchQuery Fetch () noexcept;

    /// \return Prepared query for readwrite access.
    ModifyQuery Modify () noexcept;

    /// Assigning storage handles looks counter intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (ShortTermStorage);

private:
    /// Registry constructs storages.
    friend class Registry;

    explicit ShortTermStorage (void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Warehouse