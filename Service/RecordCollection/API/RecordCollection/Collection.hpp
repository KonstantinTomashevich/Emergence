#pragma once

#include <vector>

#include <StandardLayout/Mapping.hpp>

#include <RecordCollection/Resolver.hpp>

namespace Emergence::RecordCollection
{
class Collection final
{
public:
    class Inserter final
    {
    public:
        Inserter (const Inserter &_other) = delete;

        Inserter (Inserter &&_other) = delete;

        ~Inserter ();

        void *AllocateObject ();

    private:
        /// Collection constructs inserters.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit Inserter (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    class PointResolverIterator final
    {
    public:
        ~PointResolverIterator () noexcept;

        PointResolver operator * () const noexcept;

        PointResolverIterator &operator ++ () noexcept;

        PointResolverIterator operator ++ (int) noexcept;

        PointResolverIterator &operator -- () noexcept;

        PointResolverIterator operator -- (int) noexcept;

        bool operator == (const PointResolverIterator &_other) const noexcept;

        bool operator != (const PointResolverIterator &_other) const noexcept;

    private:
        /// Collection constructs iterators for point resolvers.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit PointResolverIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    class LinearResolverIterator final
    {
    public:
        ~LinearResolverIterator () noexcept;

        LinearResolver operator * () const noexcept;

        LinearResolverIterator &operator ++ () noexcept;

        LinearResolverIterator operator ++ (int) noexcept;

        LinearResolverIterator &operator -- () noexcept;

        LinearResolverIterator operator -- (int) noexcept;

        bool operator == (const LinearResolverIterator &_other) const noexcept;

        bool operator != (const LinearResolverIterator &_other) const noexcept;

    private:
        /// Collection constructs iterators for linear resolvers.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit LinearResolverIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    class VolumetricResolverIterator final
    {
    public:
        ~VolumetricResolverIterator () noexcept;

        VolumetricResolver operator * () const noexcept;

        VolumetricResolverIterator &operator ++ () noexcept;

        VolumetricResolverIterator operator ++ (int) noexcept;

        VolumetricResolverIterator &operator -- () noexcept;

        VolumetricResolverIterator operator -- (int) noexcept;

        bool operator == (const VolumetricResolverIterator &_other) const noexcept;

        bool operator != (const VolumetricResolverIterator &_other) const noexcept;

    private:
        /// Collection constructs iterators for volumetric resolvers.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit VolumetricResolverIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    explicit Collection (StandardLayout::Mapping _typeMapping);

    Collection (const Collection &_other) = delete;

    Collection (Collection &&_other);

    ~Collection ();

    // TODO: With current interface only possible solution with inserters and cursors is to halt program if object
    //       creation can not be done. Is it OK or is it better to use optionals in such situations?
    Inserter Insert ();

    PointResolver CreatePointResolver (const std::vector <StandardLayout::FieldId> _keyFields) const;

    LinearResolver CreateLinearResolver (StandardLayout::FieldId _keyField) const;

    VolumetricResolver CreateVolumetricResolver (const std::vector <StandardLayout::FieldId> _keyFields) const;

    PointResolverIterator PointResolverBegin ();

    PointResolverIterator PointResolverEnd ();

    LinearResolverIterator LinearResolverBegin ();

    LinearResolverIterator LinearResolverEnd ();

    VolumetricResolverIterator VolumetricResolverBegin ();

    VolumetricResolverIterator VolumetricResolverEnd ();

private:
    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection
