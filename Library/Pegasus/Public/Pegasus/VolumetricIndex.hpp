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

        SupportedAxisValue globalMinBorder;

        StandardLayout::FieldId maxBorderField;

        SupportedAxisValue globalMaxBorder;
    };

    /// Axis aligned shape description for shape intersection lookups.
    struct AxisAlignedShape
    {
        std::array <SupportedAxisValue, Constants::VolumetricIndex::MAX_DIMENSIONS> min;
        std::array <SupportedAxisValue, Constants::VolumetricIndex::MAX_DIMENSIONS> max;
    };

    class ShapeIntersectionCursorBase
    {
    public:
        ShapeIntersectionCursorBase (const ShapeIntersectionCursorBase &_other) noexcept;

        ShapeIntersectionCursorBase (ShapeIntersectionCursorBase &&_other) noexcept;

        ~ShapeIntersectionCursorBase () noexcept;

    protected:
        ShapeIntersectionCursorBase (
            VolumetricIndex *_index, const LeafSector &_sector, const AxisAlignedShape &_shape) noexcept;

        bool IsFinished () const noexcept;

        void MoveToNextRecord () noexcept;

        const void *GetRecord () const noexcept;

        VolumetricIndex *GetIndex () const noexcept;

        /// If cursor is not finished and as a result of record deletion, record edition or cursor construction
        /// ::currentRecordIndex is invalid, current record is already visited or does not intersect with cursor
        /// shape, we should execute ::MoveToNextRecord.
        void FixCurrentRecordIndex () noexcept;

        template <typename Operations>
        bool CheckShapeIntersection (const void *_record, const Operations &_operations) const noexcept;

    private:
        VolumetricIndex *index;
        const LeafSector sector;
        const AxisAlignedShape shape;

        LeafCoordinate currentCoordinate;
        std::size_t currentRecordIndex;
        std::vector <bool> visitedRecords;
    };

    class ShapeIntersectionReadCursor final : private ShapeIntersectionCursorBase
    {
    public:
        ShapeIntersectionReadCursor (const ShapeIntersectionReadCursor &_other) noexcept;

        ShapeIntersectionReadCursor (ShapeIntersectionReadCursor &&_other) noexcept;

        ~ShapeIntersectionReadCursor () noexcept;

        const void *operator * () const noexcept;

        ShapeIntersectionReadCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        ShapeIntersectionReadCursor &operator = (const ShapeIntersectionReadCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ShapeIntersectionReadCursor &operator = (ShapeIntersectionReadCursor &&_other) = delete;

    private:
        friend class VolumetricIndex;

        ShapeIntersectionReadCursor (
            VolumetricIndex *_index, const LeafSector &_sector, const AxisAlignedShape &_shape) noexcept;
    };

    class ShapeIntersectionEditCursor final : private ShapeIntersectionCursorBase
    {
    public:
        ShapeIntersectionEditCursor (const ShapeIntersectionEditCursor &_other) = delete;

        ShapeIntersectionEditCursor (ShapeIntersectionEditCursor &&_other) noexcept;

        ~ShapeIntersectionEditCursor () noexcept;

        void *operator * () noexcept;

        ShapeIntersectionEditCursor &operator ~ () noexcept;

        ShapeIntersectionEditCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        ShapeIntersectionEditCursor &operator = (const ShapeIntersectionEditCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ShapeIntersectionEditCursor &operator = (ShapeIntersectionEditCursor &&_other) = delete;

    private:
        friend class VolumetricIndex;

        ShapeIntersectionEditCursor (VolumetricIndex *_index, const LeafSector &_sector,
                                     const AxisAlignedShape &_shape) noexcept;

        void BeginRecordEdition () const noexcept;
    };

    /// There is no sense to copy indices.
    VolumetricIndex (const VolumetricIndex &_other) = delete;

    /// Moving indices is forbidden, because otherwise user can move index out of Storage.
    VolumetricIndex (VolumetricIndex &&_other) = delete;

    const InplaceVector <Dimension, Constants::VolumetricIndex::MAX_DIMENSIONS> &GetDimensions () const noexcept;

    ShapeIntersectionReadCursor LookupShapeIntersectionToRead (const AxisAlignedShape &_shape) noexcept;

    ShapeIntersectionEditCursor LookupShapeIntersectionToEdit (const AxisAlignedShape &_shape) noexcept;

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

    template <typename Operations>
    LeafSector CalculateSector (const AxisAlignedShape &_shape, const Operations &_operations) const noexcept;

    template <typename Operations>
    SupportedAxisValue CalculateLeafSize (
        const Dimension &_dimension, const Operations &_operations) const noexcept;

    template <typename Operations>
    std::size_t CalculateCoordinate (
        const SupportedAxisValue &_value, const Dimension &_dimension,
        const SupportedAxisValue &_leafSize, const Operations &_operations) const noexcept;

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