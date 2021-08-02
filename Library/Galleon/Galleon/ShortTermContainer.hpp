#pragma once

#include <API/Common/Cursor.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Galleon/AccessCounter.hpp>
#include <Galleon/ContainerBase.hpp>

#include <Memory/Pool.hpp>

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

            /// \invariant Previously allocated object must be initialized before cursor destruction.
            ~Cursor () noexcept;

            /// \return Pointer to memory, allocated for the new object.
            /// \invariant Previously allocated object must be initialized before next call.
            void *operator ++ () noexcept;

            /// Assigning cursors looks counter intuitive.
            EMERGENCE_DELETE_ASSIGNMENT (Cursor);

        private:
            friend class InsertQuery;

            explicit Cursor (Handling::Handle <ShortTermContainer> _container) noexcept;

            Handling::Handle <ShortTermContainer> container;
        };

        InsertQuery (const InsertQuery &_other) noexcept = default;

        InsertQuery (InsertQuery &&_other) noexcept = default;

        ~InsertQuery () noexcept = default;

        Cursor Execute () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (InsertQuery);

    private:
        friend class ShortTermContainer;

        explicit InsertQuery (Handling::Handle <ShortTermContainer> _container) noexcept;

        Handling::Handle <ShortTermContainer> container;
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

            explicit Cursor (Handling::Handle <ShortTermContainer> _container) noexcept;

            Handling::Handle <ShortTermContainer> container;

            std::vector <void *>::const_iterator iterator;

            const std::vector <void *>::const_iterator end;
        };

        FetchQuery (const FetchQuery &_other) noexcept = default;

        FetchQuery (FetchQuery &&_other) noexcept = default;

        /// \invariant There is no cursors for this query. Otherwise cursor deletion will not be thread safe.
        ~FetchQuery () noexcept = default;

        Cursor Execute () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (FetchQuery);

    private:
        friend class ShortTermContainer;

        explicit FetchQuery (Handling::Handle <ShortTermContainer> _container) noexcept;

        Handling::Handle <ShortTermContainer> container;
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

            explicit Cursor (Handling::Handle <ShortTermContainer> _container) noexcept;

            Handling::Handle <ShortTermContainer> container;

            std::vector <void *>::iterator iterator;

            std::vector <void *>::iterator end;
        };

        ModifyQuery (const ModifyQuery &_other) noexcept = default;

        ModifyQuery (ModifyQuery &&_other) noexcept = default;

        ~ModifyQuery () noexcept = default;

        Cursor Execute () const noexcept;

        /// Assigning prepared queries looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (ModifyQuery);

    private:
        friend class ShortTermContainer;

        explicit ModifyQuery (Handling::Handle <ShortTermContainer> _container) noexcept;

        Handling::Handle <ShortTermContainer> container;
    };

    InsertQuery Insert () noexcept;

    FetchQuery Fetch () noexcept;

    ModifyQuery Modify () noexcept;

private:
    /// CargoDeck constructs containers.
    friend class CargoDeck;

    /// Only handles have right to destruct containers.
    template <typename>
    friend class Handling::Handle;

    explicit ShortTermContainer (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept;

    ~ShortTermContainer () noexcept;

    /// \brief Pool iteration could be slow, therefore we maintain additional vector of records.
    std::vector <void *> objects;

    Memory::Pool pool;

    AccessCounter accessCounter;
};
} // namespace Emergence::Galleon
