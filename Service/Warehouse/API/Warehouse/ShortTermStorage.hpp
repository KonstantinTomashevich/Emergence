#pragma once

#include <array>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Warehouse
{
class ShortTermStorage final
{
public:
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
        friend class ShortTermStorage;

        explicit InsertQuery (void *_handle) noexcept;

        void *handle;
    };

    class FetchQuery final
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

        ~FetchQuery ();

        Cursor Execute () noexcept;

    private:
        friend class ShortTermStorage;

        explicit FetchQuery (void *_handle) noexcept;

        void *handle;
    };

    class ModifyQuery final
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

        ~ModifyQuery ();

        Cursor Execute () noexcept;

    private:
        friend class ShortTermStorage;

        explicit ModifyQuery (void *_handle) noexcept;

        void *handle;
    };

    ~ShortTermStorage () noexcept;

    StandardLayout::Mapping GetTypeMapping () const noexcept;

    InsertQuery Insert () noexcept;

    FetchQuery Fetch () noexcept;

    ModifyQuery Modify () noexcept;

private:
    friend class Registry;

    explicit ShortTermStorage (void *_handle) noexcept;

    void *handle;
};
} // namespace Emergence::Warehouse