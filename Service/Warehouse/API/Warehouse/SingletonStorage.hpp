#pragma once

#include <array>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Warehouse
{
/// \brief Stores single instance of given type.
/// \details SingletonStorage object is shared-ownership handle for implementation instance. Storage will be
///          automatically destroyed if there is no handles for this storage or its prepared queries.
/// \warning After storage creation singleton instance is allocated, but not initialized. Use ModifyQuery to init it.
/// \warning Before storage destruction instance destructor must be called manually from ModifyQuery, because singleton
///          types are unknown on compile time.
class SingletonStorage final
{
public:
    /// \brief Prepared query, used to gain thread safe readonly access to singleton instance.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class FetchQuery final
    {
    public:
        /// \brief Provides thread safe readonly access to singleton instance.
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) noexcept;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            /// \return Singleton instance.
            const void *operator * () const noexcept;

            /// Assigning cursors looks counter intuitive.
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);

        private:
            /// Query constructs its cursors.
            friend class FetchQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        FetchQuery (const FetchQuery &_other) noexcept;

        FetchQuery (FetchQuery &&_other) noexcept;

        /// \invariant There is no active cursors for this query.
        ~FetchQuery () noexcept;

        /// \return Cursor, that provides thread safe readonly access to singleton instance.
        /// \details Thread safe.
        /// \invariant There is no ModifyQuery cursors in this storage.
        Cursor Execute () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchQuery);

    private:
        /// Storage constructs prepared queries.
        friend class SingletonStorage;

        explicit FetchQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    /// \brief Prepared query, used to gain readwrite access to singleton instance.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there is no handles for it.
    class ModifyQuery final
    {
    public:
        /// \brief Provides readwrite access to singleton instance.
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            /// \return Singleton instance.
            void *operator * () const noexcept;

            /// Assigning cursors looks counter intuitive.
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);

        private:
            /// Query constructs its cursors.
            friend class ModifyQuery;

            EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

            explicit Cursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;
        };

        ModifyQuery (const ModifyQuery &_other) noexcept;

        ModifyQuery (ModifyQuery &&_other) noexcept;

        /// \invariant There is no active cursors for this query.
        ~ModifyQuery () noexcept;

        /// \return Cursor, that provides readwrite access to singleton instance.
        /// \invariant There is no other cursors in this storage.
        Cursor Execute () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyQuery);

    private:
        /// Storage constructs prepared queries.
        friend class SingletonStorage;

        explicit ModifyQuery (void *_handle) noexcept;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
    };

    SingletonStorage (const SingletonStorage &_other) noexcept;

    SingletonStorage (SingletonStorage &&_other) noexcept;

    ~SingletonStorage () noexcept;

    /// \return Mapping for singleton type.
    StandardLayout::Mapping GetTypeMapping () const noexcept;

    /// \return Prepared query for readonly access.
    FetchQuery Fetch () noexcept;

    /// \return Prepared query for readwrite access.
    ModifyQuery Modify () noexcept;

    /// Assigning storage handles looks counter intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (SingletonStorage);

private:
    /// Registry constructs storages.
    friend class Registry;

    explicit SingletonStorage (void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Warehouse