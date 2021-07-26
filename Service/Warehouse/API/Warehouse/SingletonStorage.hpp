#pragma once

#include <array>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Warehouse
{
/// \brief Stores single instance of given type.
/// \details SingletonStorage object is shared-ownership handle for implementation instance. Storage will be
///          automatically destroyed if there are no handles for this storage or its prepared queries.
/// \warning After storage creation singleton instance is allocated, but not initialized. Use ModifyQuery to init it.
class SingletonStorage final
{
public:
    /// \brief Prepared query, used to gain read only access to singleton instance.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there are no handles for it.
    class FetchQuery final
    {
    public:
        /// \brief Provides thread safe read only access to singleton instance.
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) noexcept;

            Cursor (Cursor &&_other) noexcept;

            ~Cursor () noexcept;

            /// \return Singleton instance.
            const void *operator * () const noexcept;

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

        /// \invariant There is no active cursors for this query.
        ~FetchQuery () noexcept;

        /// \return Cursor, that provides thread safe read only access to singleton instance.
        /// \details Thread safe.
        /// \invariant There is no ModifyQuery cursors in this storage.
        Cursor Execute () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        FetchQuery &operator = (const FetchQuery &_other) = delete;

        /// Assigning prepared queries looks counter intuitive.
        FetchQuery &operator = (FetchQuery &&_other) = delete;

    private:
        /// Storage constructs prepared queries.
        friend class SingletonStorage;

        explicit FetchQuery (void *_handle) noexcept;

        /// \brief Implementation handle.
        void *handle;
    };

    /// \brief Prepared query, used to gain readwrite access to singleton instance.
    /// \details Object of this class is shared-ownership handle for implementation instance.
    ///          Prepared query will be automatically deallocated if there are no handles for it.
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

        /// \invariant There is no active cursors for this query.
        ~ModifyQuery () noexcept;

        /// \return Cursor, that provides readwrite access to singleton instance.
        /// \invariant There is no other cursors in this storage.
        Cursor Execute () noexcept;

        /// Assigning prepared queries looks counter intuitive.
        ModifyQuery &operator = (const ModifyQuery &_other) = delete;

        /// Assigning prepared queries looks counter intuitive.
        ModifyQuery &operator = (ModifyQuery &&_other) = delete;

    private:
        /// Storage constructs prepared queries.
        friend class SingletonStorage;

        explicit ModifyQuery (void *_handle) noexcept;

        /// \brief Implementation handle.
        void *handle;
    };

    SingletonStorage (const SingletonStorage &_other) noexcept;

    SingletonStorage (SingletonStorage &&_other) noexcept;

    ~SingletonStorage () noexcept;

    /// \return Mapping for singleton type.
    StandardLayout::Mapping GetTypeMapping () const noexcept;

    /// \return Prepared query for read only access.
    FetchQuery Fetch () noexcept;

    /// \return Prepared query for readwrite access.
    ModifyQuery Modify () noexcept;

    /// Assigning storage handles looks counter intuitive.
    SingletonStorage &operator = (const SingletonStorage &_other) = delete;

    /// Assigning storage handles looks counter intuitive.
    SingletonStorage &operator = (SingletonStorage &&_other) = delete;

private:
    /// Registry constructs storages.
    friend class Registry;

    explicit SingletonStorage (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::Warehouse