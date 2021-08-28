#pragma once

#include <vector>

#include <API/Common/Cursor.hpp>

#include <Handling/HandleableBase.hpp>

#include <Pegasus/Constants/VolumetricIndex.hpp>
#include <Pegasus/IndexBase.hpp>

#include <StandardLayout/Field.hpp>

#include <SyntaxSugar/InplaceVector.hpp>

namespace Emergence::Pegasus
{
class VolumetricIndex final : public IndexBase
{
private:
    using LeafCoordinate = std::array<std::size_t, Constants::VolumetricIndex::MAX_DIMENSIONS>;

    struct LeafSector final
    {
        LeafCoordinate min;
        LeafCoordinate max;
    };

public:
    union SupportedAxisValue final
    {
        SupportedAxisValue () noexcept;

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
    struct DimensionDescriptor final
    {
        SupportedAxisValue globalMinBorder;

        StandardLayout::FieldId minBorderField;

        SupportedAxisValue globalMaxBorder;

        StandardLayout::FieldId maxBorderField;
    };

    /// \brief Axis aligned shape with specific value type.
    template <typename AxisValue>
    struct AxisAlignedShape final
    {
        /// \brief Contains min-max pairs for all dimensions.
        /// \details It's recommended to use ::Min and ::Max methods to access values of this array.
        std::array<AxisValue, Constants::VolumetricIndex::MAX_DIMENSIONS * 2u> data;

        AxisValue &Min (std::size_t _dimensionIndex) noexcept;

        AxisValue &Max (std::size_t _dimensionIndex) noexcept;

        const AxisValue &Min (std::size_t _dimensionIndex) const noexcept;

        const AxisValue &Max (std::size_t _dimensionIndex) const noexcept;
    };

    /// \brief Memory block, that can fit AxisAlignedShape of any supported type.
    using AxisAlignedShapeContainer = std::array<uint8_t, sizeof (AxisAlignedShape<SupportedAxisValue>)>;

    /// \brief Ray with specific value type.
    template <typename AxisValue>
    struct Ray final
    {
        /// \brief Contains origin-direction pairs for all dimensions.
        /// \details It's recommended to use ::Origin and ::Direction methods to access values of this array.
        std::array<AxisValue, Constants::VolumetricIndex::MAX_DIMENSIONS * 2u> data;

        AxisValue &Origin (std::size_t _dimensionIndex) noexcept;

        AxisValue &Direction (std::size_t _dimensionIndex) noexcept;

        const AxisValue &Origin (std::size_t _dimensionIndex) const noexcept;

        const AxisValue &Direction (std::size_t _dimensionIndex) const noexcept;
    };

    /// \brief Memory block, that can fit Ray of any supported type.
    using RayContainer =
        std::array<uint8_t, sizeof (SupportedAxisValue) * Constants::VolumetricIndex::MAX_DIMENSIONS * 2u>;

    class ShapeIntersectionCursorBase
    {
    public:
        ShapeIntersectionCursorBase (const ShapeIntersectionCursorBase &_other) noexcept;

        ShapeIntersectionCursorBase (ShapeIntersectionCursorBase &&_other) noexcept;

        ~ShapeIntersectionCursorBase () noexcept;

    protected:
        ShapeIntersectionCursorBase (VolumetricIndex *_index,
                                     const LeafSector &_sector,
                                     const AxisAlignedShapeContainer &_shape) noexcept;

        bool IsFinished () const noexcept;

        void MoveToNextRecord () noexcept;

        const void *GetRecord () const noexcept;

        VolumetricIndex *GetIndex () const noexcept;

        /// Check CursorCommons::FixCurrentRecordIndex for explanation.
        void FixCurrentRecordIndex () noexcept;

    private:
        template <typename>
        friend struct CursorCommons;

        template <typename Operations>
        bool MoveToNextCoordinate (const Operations &) noexcept;

        template <typename Operations>
        bool CheckIntersection (const void *_record, const Operations &_operations) const noexcept;

        VolumetricIndex *index;
        const LeafSector sector;
        const AxisAlignedShapeContainer shape;

        LeafCoordinate currentCoordinate;
        std::size_t currentRecordIndex;
        std::vector<bool> visitedRecords;
    };

    class ShapeIntersectionReadCursor final : private ShapeIntersectionCursorBase
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (ShapeIntersectionReadCursor);

    private:
        friend class VolumetricIndex;

        ShapeIntersectionReadCursor (VolumetricIndex *_index,
                                     const LeafSector &_sector,
                                     const AxisAlignedShapeContainer &_shape) noexcept;
    };

    class ShapeIntersectionEditCursor final : private ShapeIntersectionCursorBase
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (ShapeIntersectionEditCursor);

    private:
        friend class VolumetricIndex;

        ShapeIntersectionEditCursor (VolumetricIndex *_index,
                                     const LeafSector &_sector,
                                     const AxisAlignedShapeContainer &_shape) noexcept;

        void BeginRecordEdition () const noexcept;
    };

    class RayIntersectionCursorBase
    {
    public:
        RayIntersectionCursorBase (const RayIntersectionCursorBase &_other) noexcept;

        RayIntersectionCursorBase (RayIntersectionCursorBase &&_other) noexcept;

        ~RayIntersectionCursorBase () noexcept;

    protected:
        RayIntersectionCursorBase (VolumetricIndex *_index, const RayContainer &_ray, float _maxDistance) noexcept;

        bool IsFinished () const noexcept;

        void MoveToNextRecord () noexcept;

        const void *GetRecord () const noexcept;

        VolumetricIndex *GetIndex () const noexcept;

        /// Check CursorCommons::FixCurrentRecordIndex for explanation.
        void FixCurrentRecordIndex () noexcept;

    private:
        template <typename>
        friend struct CursorCommons;

        template <typename Operations>
        bool MoveToNextCoordinate (const Operations &) noexcept;

        template <typename Operations>
        bool CheckIntersection (const void *_record, const Operations &_operations) const noexcept;

        VolumetricIndex *index;

        /// \brief Intermediate point in leaf coordinates (not world coordinates), used for next leaf selection.
        std::array<float, Constants::VolumetricIndex::MAX_DIMENSIONS> currentPoint;

        /// \brief Ray direction, converted into leaf coordinates.
        std::array<float, Constants::VolumetricIndex::MAX_DIMENSIONS> direction;

        /// Distance from ray origin to ::currentPoint border in world coordinates.
        float distanceTraveled;

        const RayContainer ray;

        /// \brief Maximum allowed distance for ray-shape collisions.
        const float maxDistance;

        LeafCoordinate currentCoordinate;
        std::size_t currentRecordIndex;
        std::vector<bool> visitedRecords;
    };

    class RayIntersectionReadCursor final : private RayIntersectionCursorBase
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (RayIntersectionReadCursor);

    private:
        friend class VolumetricIndex;

        RayIntersectionReadCursor (VolumetricIndex *_index, const RayContainer &_ray, float _maxDistance) noexcept;
    };

    class RayIntersectionEditCursor final : private RayIntersectionCursorBase
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (RayIntersectionEditCursor);

    private:
        friend class VolumetricIndex;

        RayIntersectionEditCursor (VolumetricIndex *_index, const RayContainer &_ray, float _maxDistance) noexcept;

        void BeginRecordEdition () const noexcept;
    };

    /// There is no sense to copy indices.
    VolumetricIndex (const VolumetricIndex &_other) = delete;

    /// Moving indices is forbidden, because otherwise user can move index out of Storage.
    VolumetricIndex (VolumetricIndex &&_other) = delete;

    const InplaceVector<Dimension, Constants::VolumetricIndex::MAX_DIMENSIONS> &GetDimensions () const noexcept;

    ShapeIntersectionReadCursor LookupShapeIntersectionToRead (const AxisAlignedShapeContainer &_shape) noexcept;

    ShapeIntersectionEditCursor LookupShapeIntersectionToEdit (const AxisAlignedShapeContainer &_shape) noexcept;

    RayIntersectionReadCursor LookupRayIntersectionToRead (
        const RayContainer &_ray, float _maxDistance = std::numeric_limits<float>::max ()) noexcept;

    RayIntersectionEditCursor LookupRayIntersectionToEdit (
        const RayContainer &_ray, float _maxDistance = std::numeric_limits<float>::max ()) noexcept;

    void Drop () noexcept;

    /// There is no sense to copy assign indices.
    VolumetricIndex &operator= (const VolumetricIndex &_other) = delete;

    /// Move assigning indices is forbidden, because otherwise user can move index out of Storage.
    VolumetricIndex &operator= (VolumetricIndex &&_other) = delete;

private:
    friend class Storage;

    template <typename>
    friend struct CursorCommons;

    struct RecordData final
    {
        const void *record;
        std::size_t recordId;
    };

    struct LeafData final
    {
        std::vector<RecordData> records;

        std::vector<RecordData>::iterator FindRecord (const void *_record) noexcept;

        /// \brief Deletes record from leaf using "exchange with last" strategy.
        void DeleteRecord (const std::vector<RecordData>::iterator &_recordIterator) noexcept;
    };

    VolumetricIndex (Storage *_storage, const std::vector<DimensionDescriptor> &_dimensions) noexcept;

    template <typename Operations>
    LeafSector CalculateSector (const void *_record, const Operations &_operations) const noexcept;

    template <typename Operations>
    LeafSector CalculateSector (const AxisAlignedShapeContainer &_shape, const Operations &_operations) const noexcept;

    template <typename Operations>
    SupportedAxisValue CalculateLeafSize (const Dimension &_dimension, const Operations &_operations) const noexcept;

    template <typename Operations>
    std::size_t CalculateCoordinate (const SupportedAxisValue &_value,
                                     const Dimension &_dimension,
                                     const SupportedAxisValue &_leafSize,
                                     const Operations &_operations) const noexcept;

    template <typename Operations>
    bool CheckRayShapeIntersection (
        const RayContainer &_ray,
        const AxisAlignedShapeContainer &_shape,
        float &_distanceOutput,
        std::array<float, Constants::VolumetricIndex::MAX_DIMENSIONS> &_intersectionPointOutput,
        const Operations &_operations) const noexcept;

    template <typename Callback>
    void ForEachCoordinate (const LeafSector &_sector, const Callback &_callback) const noexcept;

    std::size_t GetLeafIndex (const LeafCoordinate &_coordinate) const noexcept;

    bool IsInsideSector (const LeafSector &_sector, const LeafCoordinate &_coordinate) const noexcept;

    bool AreEqual (const LeafCoordinate &_left, const LeafCoordinate &_right) const noexcept;

    VolumetricIndex::LeafCoordinate NextInsideSector (const LeafSector &_sector,
                                                      LeafCoordinate _coordinate) const noexcept;

    bool AreEqual (const LeafSector &_left, const LeafSector &_right) const noexcept;

    void InsertRecord (const void *_record) noexcept;

    void OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    void OnWriterClosed () noexcept;

    InplaceVector<Dimension, Constants::VolumetricIndex::MAX_DIMENSIONS> dimensions;

    // TODO: For now we use simplified approach and work only with leaves, therefore reducing octree into grid.
    //       This approach is problematic because it significantly limits count of subdivisions.
    std::vector<LeafData> leaves;
    std::vector<std::size_t> freeRecordIds;
    std::size_t nextRecordId;
};

template <typename AxisValue>
AxisValue &VolumetricIndex::AxisAlignedShape<AxisValue>::Min (std::size_t _dimensionIndex) noexcept
{
    return data[_dimensionIndex * 2u];
}

template <typename AxisValue>
AxisValue &VolumetricIndex::AxisAlignedShape<AxisValue>::Max (std::size_t _dimensionIndex) noexcept
{
    return data[_dimensionIndex * 2u + 1u];
}

template <typename AxisValue>
const AxisValue &VolumetricIndex::AxisAlignedShape<AxisValue>::Min (std::size_t _dimensionIndex) const noexcept
{
    return data[_dimensionIndex * 2u];
}

template <typename AxisValue>
const AxisValue &VolumetricIndex::AxisAlignedShape<AxisValue>::Max (std::size_t _dimensionIndex) const noexcept
{
    return data[_dimensionIndex * 2u + 1u];
}

template <typename AxisValue>
AxisValue &VolumetricIndex::Ray<AxisValue>::Origin (std::size_t _dimensionIndex) noexcept
{
    return data[_dimensionIndex * 2u];
}

template <typename AxisValue>
AxisValue &VolumetricIndex::Ray<AxisValue>::Direction (std::size_t _dimensionIndex) noexcept
{
    return data[_dimensionIndex * 2u + 1u];
}

template <typename AxisValue>
const AxisValue &VolumetricIndex::Ray<AxisValue>::Origin (std::size_t _dimensionIndex) const noexcept
{
    return data[_dimensionIndex * 2u];
}

template <typename AxisValue>
const AxisValue &VolumetricIndex::Ray<AxisValue>::Direction (std::size_t _dimensionIndex) const noexcept
{
    return data[_dimensionIndex * 2u + 1u];
}
} // namespace Emergence::Pegasus