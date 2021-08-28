#pragma once

#include <cstdint>

#include <API/Common/Shortcuts.hpp>

#include <Galleon/AccessCounter.hpp>
#include <Galleon/ContainerBase.hpp>

namespace Emergence::Galleon
{
/// \brief Stores single instance of given type.
/// \warning After container creation singleton instance is allocated, but not initialized. Use ModifyQuery to init it.
/// \warning Before container destruction instance destructor must be called manually from ModifyQuery,
///          because singleton types are unknown on compile time.
class SingletonContainer final : public ContainerBase
{
public:
    /// \brief Provides thread safe read only access to singleton instance.
    class FetchQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) noexcept;

            Cursor (Cursor &&_other) noexcept = default;

            ~Cursor () noexcept;

            /// \return Singleton instance.
            const void *operator* () const noexcept;

            /// Assigning cursors looks counter intuitive.
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);

        private:
            friend class FetchQuery;

            explicit Cursor (Handling::Handle<SingletonContainer> _container) noexcept;

            Handling::Handle<SingletonContainer> container;
        };

        FetchQuery (const FetchQuery &_other) noexcept = default;

        FetchQuery (FetchQuery &&_other) noexcept = default;

        /// \invariant There is no cursors for this query. Otherwise cursor deletion will not be thread safe.
        ~FetchQuery () noexcept = default;

        Cursor Execute () const noexcept;

        Handling::Handle<SingletonContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchQuery);

    private:
        friend class SingletonContainer;

        explicit FetchQuery (Handling::Handle<SingletonContainer> _container) noexcept;

        Handling::Handle<SingletonContainer> container;
    };

    /// \brief Provides readwrite access to singleton instance.
    class ModifyQuery final
    {
    public:
        class Cursor final
        {
        public:
            Cursor (const Cursor &_other) = delete;

            Cursor (Cursor &&_other) noexcept = default;

            ~Cursor () noexcept;

            /// \return Singleton instance.
            void *operator* () const noexcept;

            /// Assigning cursors looks counter intuitive.
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);

        private:
            friend class ModifyQuery;

            explicit Cursor (Handling::Handle<SingletonContainer> _container) noexcept;

            Handling::Handle<SingletonContainer> container;
        };

        ModifyQuery (const ModifyQuery &_other) noexcept = default;

        ModifyQuery (ModifyQuery &&_other) noexcept = default;

        ~ModifyQuery () noexcept = default;

        Cursor Execute () const noexcept;

        Handling::Handle<SingletonContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyQuery);

    private:
        friend class SingletonContainer;

        explicit ModifyQuery (Handling::Handle<SingletonContainer> _container) noexcept;

        Handling::Handle<SingletonContainer> container;
    };

    FetchQuery Fetch () noexcept;

    ModifyQuery Modify () noexcept;

private:
    /// CargoDeck constructs containers.
    friend class CargoDeck;

    /// Only handles have right to destruct containers.
    template <typename>
    friend class Handling::Handle;

    /// \warning Must be used in pair with custom ::new.
    explicit SingletonContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept;

    /// \warning Must be used in pair with custom ::delete.
    ~SingletonContainer () noexcept;

    /// \brief We store singleton instance inside container, therefore we need custom allocator to do this.
    void *operator new (std::size_t, const StandardLayout::Mapping &_typeMapping) noexcept;

    /// \brief Needed because of custom ::new.
    void operator delete (void *_pointer) noexcept;

    AccessCounter accessCounter;

    uint8_t storage[0u];
};
} // namespace Emergence::Galleon