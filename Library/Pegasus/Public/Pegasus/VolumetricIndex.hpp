#pragma once

#include <Handling/HandleableBase.hpp>

#include <Pegasus/Constants/VolumetricIndex.hpp>
#include <Pegasus/IndexBase.hpp>

#include <StandardLayout/Field.hpp>

namespace Emergence::Pegasus
{
class VolumetricIndex final : public IndexBase
{
private:
    using LeafCoordinate = std::array <std::size_t, Constants::VolumetricIndex::MAX_DIMENSIONS>;

    struct LeafSector
    {
        LeafCoordinate min;
        LeafCoordinate max;
    };

public:
    union SupportedAxisValue
    {
        SupportedAxisValue (int8_t _value) noexcept;

        SupportedAxisValue (int16_t _value) noexcept;

        SupportedAxisValue (int32_t _value) noexcept;

        SupportedAxisValue (int64_t _value) noexcept;

        SupportedAxisValue (uint8_t _value) noexcept;

        SupportedAxisValue (uint16_t _value) noexcept;

        SupportedAxisValue (uint32_t _value) noexcept;

        SupportedAxisValue (uint64_t _value) noexcept;

        SupportedAxisValue (float _value) noexcept;

        SupportedAxisValue (double _value) noexcept;

        int8_t int8;
        int16_t int16;
        int32_t int32;
        int64_t int64;

        uint8_t uint8;
        uint16_t uint16;
        uint32_t uint32;
        uint64_t uint64;

        float floating;
        double doubleFloating;
    };

    struct Dimension
    {
        StandardLayout::Field minBorderField;

        SupportedAxisValue globalMinBorder = 0u;

        StandardLayout::Field maxBorderField;

        SupportedAxisValue globalMaxBorder = 0u;
    };

    /// Used only for index construction. ::Dimension is according dimension representation of constructed index.
    struct DimensionDescriptor
    {
        StandardLayout::FieldId minBorderField;

        const void *globalMinBorder;

        StandardLayout::FieldId maxBorderField;

        const void *globalMaxBorder;
    };

    class CursorBase
    {
    public:
        CursorBase (const CursorBase &_other) noexcept;

        CursorBase (CursorBase &&_other) noexcept;

        ~CursorBase () noexcept;

    protected:
        friend class VolumetricIndex;

        CursorBase (VolumetricIndex *_index, LeafSector _sector,
                    LeafCoordinate _currentCoordinate, size_t _currentRecordIndex) noexcept;

        bool IsFinished () const noexcept;

        void MoveToNextRecord () noexcept;

        const void *GetRecord () const noexcept;

    private:
        // Record deletion is done using "exchange with last" strategy, therefore ::currentRecordIndex
        // can be invalidated only if it pointed to last record in leaf. This method fixes ::currentRecordIndex
        // if this situation is detected.
        void FixCurrentRecordIndex () noexcept;

        VolumetricIndex *index;
        LeafSector sector;
        LeafCoordinate currentCoordinate;
        std::size_t currentRecordIndex;
        std::vector <bool> visitedRecords;
    };

    const InplaceVector <Dimension, Constants::VolumetricIndex::MAX_DIMENSIONS> &GetDimensions () const noexcept;

    void Drop () noexcept;

private:
    friend class Storage;

    struct RecordData
    {
        const void *record;
        std::size_t recordId;
    };

    struct LeafData
    {
        std::vector <RecordData> records;

        std::vector <RecordData>::iterator FindRecord (const void *_record) noexcept;

        /// \brief Deletes record from leaf using "exchange with last" strategy.
        void DeleteRecord (const std::vector <RecordData>::iterator &_recordIterator) noexcept;
    };

    VolumetricIndex (Storage *_storage, const std::vector <DimensionDescriptor> &_dimensions) noexcept;

    template <typename Operations>
    LeafSector CalculateSector (const void *_record, const Operations &_operations) const noexcept;

    template <typename Callback>
    void ForEachCoordinate (const LeafSector &_sector, const Callback &_callback) const noexcept;

    std::size_t GetLeafIndex (const LeafCoordinate &_coordinate) const noexcept;

    bool IsInsideSector (const LeafSector &_sector, const LeafCoordinate &_coordinate) const noexcept;

    bool AreEqual (const LeafCoordinate &_left, const LeafCoordinate &_right) const noexcept;

    VolumetricIndex::LeafCoordinate NextInsideSector (
        const LeafSector &_sector, LeafCoordinate _coordinate) const noexcept;

    bool AreEqual (const LeafSector &_left, const LeafSector &_right) const noexcept;

    void InsertRecord (const void *_record) noexcept;

    void OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept;

    void DeleteRecordMyself (CursorBase &_cursor) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    void OnWriterClosed () noexcept;

    InplaceVector <Dimension, Constants::VolumetricIndex::MAX_DIMENSIONS> dimensions;

    // TODO: For now we use simplified approach and work only with leaves, therefore reducing octree into grid.
    //       This approach is problematic because it significantly limits count of subdivisions.
    std::vector <LeafData> leaves;
    std::vector <std::size_t> freeRecordIds;
    std::size_t nextRecordId;
};
} // namespace Emergence::Pegasus