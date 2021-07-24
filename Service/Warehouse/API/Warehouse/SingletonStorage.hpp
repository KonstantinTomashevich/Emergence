#pragma once

#include <array>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Warehouse
{
class SingletonStorage final
{
public:
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
        friend class SingletonStorage;

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
        friend class SingletonStorage;

        explicit ModifyQuery (void *_handle) noexcept;

        void *handle;
    };

    ~SingletonStorage () noexcept;

    StandardLayout::Mapping GetTypeMapping () const noexcept;

    FetchQuery Fetch () noexcept;

    ModifyQuery Modify () noexcept;

private:
    friend class Registry;

    explicit SingletonStorage (void *_handle) noexcept;

    void *handle;
};
} // namespace Emergence::Warehouse