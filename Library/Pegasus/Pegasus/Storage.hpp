#pragma once

#include <array>
#include <atomic>
#include <vector>

#include <Handling/Handle.hpp>

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
    ~Storage () noexcept;

    const std::vector <Handling::Handle <HashIndex>> &GetHashIndices () noexcept;

    const std::vector <Handling::Handle <HashIndex>> &GetOrderedIndices () noexcept;

    const std::vector <Handling::Handle <HashIndex>> &GetVolumetricIndices () noexcept;

private:
    static constexpr std::size_t MAX_INDEXED_FIELDS = 32u;

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
        std::vector <Handling::Handle <HashIndex>> hash;
        std::vector <Handling::Handle <OrderedIndex>> ordered;
        std::vector <Handling::Handle <VolumetricIndex>> volumetric;
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
        std::atomic_size_t writers = 0u;
    } accessCounter;

    struct
    {
    private:
        friend class Storage;

        std::vector <ChangedRecord> changedRecords;
        void *recordBuffer = nullptr;
    } editionState;
};
} // namespace Emergence::Pegasus