#pragma once

#include <API/Common/Cursor.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Container/TypedOrderedPool.hpp>
#include <Container/Vector.hpp>

#include <Galleon/AccessCounter.hpp>
#include <Galleon/ContainerBase.hpp>

#include <Memory/UnorderedPool.hpp>

namespace Emergence::Galleon
{
/// \brief Container for objects that are created and destroyed frequently.
class ShortTermContainer final : public ContainerBase
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

            ~Cursor () noexcept;

            void *operator++ () noexcept;

            /// Assigning cursors looks counter intuitive.
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);

        private:
            friend class InsertQuery;

            explicit Cursor (Handling::Handle<ShortTermContainer> _container) noexcept;

            Handling::Handle<ShortTermContainer> container;
        };

        InsertQuery (const InsertQuery &_other) noexcept = default;

        InsertQuery (InsertQuery &&_other) noexcept = default;

        ~InsertQuery () noexcept = default;

        [[nodiscard]] Cursor Execute () const noexcept;

        [[nodiscard]] Handling::Handle<ShortTermContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (InsertQuery);

    private:
        friend class ShortTermContainer;

        explicit InsertQuery (Handling::Handle<ShortTermContainer> _container) noexcept;

        Handling::Handle<ShortTermContainer> container;
    };

    /// \brief Prepared query, used to gain thread safe readonly access to stored objects.
    class FetchQuery final
    {
    public:
        class Cursor final
        {
        public:
            EMERGENCE_READ_CURSOR_OPERATIONS (Cursor);

        private:
            friend class FetchQuery;

            explicit Cursor (Handling::Handle<ShortTermContainer> _container) noexcept;

            Handling::Handle<ShortTermContainer> container;

            void *currentNode = nullptr;
        };

        FetchQuery (const FetchQuery &_other) noexcept = default;

        FetchQuery (FetchQuery &&_other) noexcept = default;

        /// \invariant There is no cursors for this query. Otherwise cursor deletion will not be thread safe.
        ~FetchQuery () noexcept = default;

        [[nodiscard]] Cursor Execute () const noexcept;

        [[nodiscard]] Handling::Handle<ShortTermContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchQuery);

    private:
        friend class ShortTermContainer;

        explicit FetchQuery (Handling::Handle<ShortTermContainer> _container) noexcept;

        Handling::Handle<ShortTermContainer> container;
    };

    /// \brief Prepared query, used to gain readwrite access to stored objects.
    class ModifyQuery final
    {
    public:
        class Cursor final
        {
        public:
            EMERGENCE_EDIT_CURSOR_OPERATIONS (Cursor);

        private:
            friend class ModifyQuery;

            explicit Cursor (Handling::Handle<ShortTermContainer> _container) noexcept;

            Handling::Handle<ShortTermContainer> container;

            void *currentNode = nullptr;
            void *previousNode = nullptr;
        };

        ModifyQuery (const ModifyQuery &_other) noexcept = default;

        ModifyQuery (ModifyQuery &&_other) noexcept = default;

        ~ModifyQuery () noexcept = default;

        [[nodiscard]] Cursor Execute () const noexcept;

        [[nodiscard]] Handling::Handle<ShortTermContainer> GetContainer () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyQuery);

    private:
        friend class ShortTermContainer;

        explicit ModifyQuery (Handling::Handle<ShortTermContainer> _container) noexcept;

        Handling::Handle<ShortTermContainer> container;
    };

    ShortTermContainer (const ShortTermContainer &_other) = delete;

    ShortTermContainer (ShortTermContainer &&_other) = delete;

    InsertQuery Insert () noexcept;

    FetchQuery Fetch () noexcept;

    ModifyQuery Modify () noexcept;

    void LastReferenceUnregistered () noexcept;

    void SetUnsafeFetchAllowed (bool _allowed) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ShortTermContainer);

private:
    /// Pool from CargoDeck constructs and destructs containers.
    template <typename Item>
    friend class Container::TypedOrderedPool;

    static void *GetNodeContent (void *_node) noexcept;

    static const void *GetNodeContent (const void *_node) noexcept;

    explicit ShortTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept;

    ~ShortTermContainer () noexcept;

    const void *GetNextNode (const void *_node) noexcept;

    void *GetNextNode (void *_node) noexcept;

    void SetNextNode (void *_node, void *_next) noexcept;

    Memory::UnorderedPool pool;

    void *firstNode = nullptr;

    AccessCounter accessCounter;
};
} // namespace Emergence::Galleon
