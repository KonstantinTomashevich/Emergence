#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <vector>

#include <Memory/Pool.hpp>

#include <Pegasus/HashIndex.hpp>
#include <Pegasus/OrderedIndex.hpp>
#include <Pegasus/VolumetricIndex.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Pegasus
{
class Storage final
{
public:
    class Allocator final
    {
    public:
        Allocator (const Allocator &_other) = delete;

        Allocator (Allocator &&_other) noexcept;

        ~Allocator () noexcept;

        void *Next () noexcept;

        Allocator &operator = (const Allocator &_other) = delete;

        Allocator &operator = (Allocator &&_other) = delete;

    private:
        friend class Storage;

        explicit Allocator (Storage *_owner);

        Storage *owner;
        void *current = nullptr;
    };

    explicit Storage (StandardLayout::Mapping _recordMapping) noexcept;

    Storage (const Storage &_other) = delete;

    Storage (Storage &&_other) noexcept;

    ~Storage () noexcept;

    const std::vector <std::unique_ptr <HashIndex>> &GetHashIndices () noexcept;

    const std::vector <std::unique_ptr <OrderedIndex>> &GetOrderedIndices () noexcept;

    const std::vector <std::unique_ptr <VolumetricIndex>> &GetVolumetricIndices () noexcept;

private:
    friend class Allocator;

    static constexpr std::size_t MAX_INDEXED_FIELDS = 32u;

    void RegisterReader () noexcept;

    void RegisterWriter () noexcept;

    void UnregisterReader () noexcept;

    void UnregisterWriter () noexcept;

    void *AllocateRecord () noexcept;

    void InsertRecord (const void *record) noexcept;

    void BeginRecordEdition (const void *record) noexcept;

    void EndRecordEdition (const void *record) noexcept;

    struct IndexedField final
    {
        StandardLayout::Field field;
        uintptr_t usages = 0u;
    };

    struct ChangedRecord final
    {
        const void *record = nullptr;
        uint32_t changedIndexedFields = 0u;

        static_assert (sizeof (changedIndexedFields) * 8u >= MAX_INDEXED_FIELDS);
    };

    Memory::Pool records;

    struct
    {
        std::vector <std::unique_ptr <HashIndex>> hash;
        std::vector <std::unique_ptr <OrderedIndex>> ordered;
        std::vector <std::unique_ptr <VolumetricIndex>> volumetric;
    } indices;

    struct
    {
        StandardLayout::Mapping recordMapping;
        uintptr_t indexedFieldsCount = 0u;
        std::array <IndexedField, MAX_INDEXED_FIELDS> indexedFields;
    } reflection;

    struct
    {
        std::atomic_size_t readers = 0u;
        std::size_t writers = 0u;
    } accessCounter;

    struct
    {
        std::vector <ChangedRecord> changedRecords;
        void *recordBuffer = nullptr;
    } editionState;
};
} // namespace Emergence::Pegasus