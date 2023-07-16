#pragma once

#include <cstdint>

#include <API/Common/Shortcuts.hpp>

#include <Container/TypedOrderedPool.hpp>

#include <Galleon/AccessCounter.hpp>
#include <Galleon/ContainerBase.hpp>

namespace Emergence::Galleon
{
/// \brief Stores single instance of given type.
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

        [[nodiscard]] Cursor Execute () const noexcept;

        [[nodiscard]] Handling::Handle<SingletonContainer> GetContainer () const noexcept;

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

        [[nodiscard]] Cursor Execute () const noexcept;

        [[nodiscard]] Handling::Handle<SingletonContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyQuery);

    private:
        friend class SingletonContainer;

        explicit ModifyQuery (Handling::Handle<SingletonContainer> _container) noexcept;

        Handling::Handle<SingletonContainer> container;
    };

    SingletonContainer (const SingletonContainer &_other) = delete;

    SingletonContainer (SingletonContainer &&_other) = delete;

    FetchQuery Fetch () noexcept;

    ModifyQuery Modify () noexcept;

    void LastReferenceUnregistered () noexcept;

    void SetUnsafeFetchAllowed (bool _allowed) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (SingletonContainer);

private:
    /// Pool from CargoDeck constructs and destructs containers.
    template <typename Item>
    friend class Container::TypedOrderedPool;

    /// \warning Must be used in pair with custom ::new.
    explicit SingletonContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept;

    ~SingletonContainer () noexcept;

    AccessCounter accessCounter;

    void *singletonInstance;

    Memory::Heap singletonHeap;
};
} // namespace Emergence::Galleon
