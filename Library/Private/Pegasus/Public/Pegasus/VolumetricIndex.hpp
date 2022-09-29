#pragma once

#include <API/Common/Cursor.hpp>

#include <Container/InplaceVector.hpp>
#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Handling/HandleableBase.hpp>

#include <Memory/OrderedPool.hpp>

#include <Pegasus/Constants/VolumetricIndex.hpp>
#include <Pegasus/IndexBase.hpp>

namespace Emergence::Pegasus
{
using namespace Memory::Literals;

template <std::size_t Dimensions>
class PartitioningTree final
{
private:
    struct Node;

public:
    using Index = std::uint16_t;
    using FloatingIndex = float;

    struct Shape final
    {
        struct MinMax final
        {
            Index min = 0u;
            Index max = 0u;

            bool operator== (const MinMax &_other) const = default;

            bool operator!= (const MinMax &_other) const = default;
        };

        std::array<MinMax, Dimensions> bounds;

        bool operator== (const Shape &_other) const = default;

        bool operator!= (const Shape &_other) const = default;
    };

    struct Ray final
    {
        struct Axe final
        {
            FloatingIndex origin = 0.0f;
            FloatingIndex direction = 0.0f;
        };

        std::array<Axe, Dimensions> axis;
    };

    class ShapeEnumerator final
    {
    public:
        ShapeEnumerator (const ShapeEnumerator &_other) noexcept = default;

        ShapeEnumerator (ShapeEnumerator &&_other) noexcept = default;

        ~ShapeEnumerator () noexcept = default;

        /// \warning Invalidates other enumerators.
        void EraseRecord (std::size_t _index) noexcept;

        [[nodiscard]] const Container::Vector<const void *> *operator* () const noexcept;

        ShapeEnumerator &operator++ () noexcept;

        ShapeEnumerator &operator= (const ShapeEnumerator &_other) noexcept = default;

        ShapeEnumerator &operator= (ShapeEnumerator &&_other) noexcept = default;

    private:
        friend class PartitioningTree;

        struct StackItem final
        {
            Node *node = nullptr;
            Index filterValue = 0u;
            Index filterMask = 0u;
            Index nextChildToVisit = 0u;
        };

        ShapeEnumerator (PartitioningTree *_tree, const Shape &_shape) noexcept;

        void EnterNode (Node *_node) noexcept;

        PartitioningTree *tree = nullptr;
        const Shape shape;
        Container::InplaceVector<StackItem, Constants::VolumetricIndex::MAX_LEVELS> stack;
    };

    PartitioningTree (const std::array<Index, Dimensions> &_borders) noexcept;

    PartitioningTree (const PartitioningTree &_other) = delete;

    PartitioningTree (PartitioningTree &&_other) noexcept;

    ~PartitioningTree () noexcept;

    [[nodiscard]] const std::array<Index, Dimensions> &GetBorders () const noexcept;

    ShapeEnumerator EnumerateIntersectingShapes (const Shape &_shape) noexcept;

    /// \warning Invalidates iterators.
    void Insert (const void *_record, const Shape &_shape);

    /// \warning Invalidates iterators.
    void Erase (const void *_record, const Shape &_shape) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PartitioningTree);

private:
    static_assert (Dimensions < sizeof (Index) * 8u);

    static constexpr Index NODE_CHILDREN_COUNT = 1u << Dimensions;

    static constexpr Index SELECT_TOP_NODE = NODE_CHILDREN_COUNT;

    struct Node final
    {
        Node (PartitioningTree *_tree, const std::array<Index, Dimensions> &_center) noexcept;

        Node (const Node &_other) = delete;

        Node (Node &&_other) = delete;

        ~Node () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (Node);

        Container::Vector<const void *> records;
        std::array<Index, Dimensions> center;

        /// \brief We store children count to make no-children check faster.
        std::size_t childrenCount = 0u;

        std::array<Node *, NODE_CHILDREN_COUNT> children;
    };

    static std::size_t SelectNodeChildForShape (const Node &_node, const Shape &_shape) noexcept;

    static bool IsSafeToDelete (const Node &_node) noexcept;

    void DeleteNodeWithChildren (Node *_node);

    std::array<Index, Dimensions> borders;
    Index maxLevel = 1u;
    Memory::OrderedPool nodePool {Memory::Profiler::AllocationGroup {"Node"_us}, sizeof (Node), alignof (Node)};
    Node *root = nullptr;
};

template <typename Unit, std::size_t Dimensions>
class VolumetricTree final
{
public:
    struct Shape final
    {
        struct MinMax final
        {
            Unit min = 0u;
            Unit max = 0u;
        };

        std::array<MinMax, Dimensions> bounds;
    };

    struct Ray final
    {
        struct Axe final
        {
            Unit origin = 0.0f;
            Unit direction = 0.0f;
        };

        std::array<Axe, Dimensions> axis;
    };

    struct Dimension final
    {
        StandardLayout::Field minField;

        Unit minBorder;

        StandardLayout::Field maxField;

        Unit maxBorder;
    };

    class ShapeIntersectionEnumerator final
    {
    public:
        ShapeIntersectionEnumerator (const ShapeIntersectionEnumerator &_other) noexcept = default;

        ShapeIntersectionEnumerator (ShapeIntersectionEnumerator &&_other) noexcept = default;

        ~ShapeIntersectionEnumerator () noexcept = default;

        [[nodiscard]] const void *operator* () const noexcept;

        ShapeIntersectionEnumerator &operator++ () noexcept;

        ShapeIntersectionEnumerator &operator~() noexcept;

        ShapeIntersectionEnumerator &operator= (const ShapeIntersectionEnumerator &_other) noexcept = default;

        ShapeIntersectionEnumerator &operator= (ShapeIntersectionEnumerator &&_other) noexcept = default;

    private:
        friend class VolumetricTree;

        ShapeIntersectionEnumerator (VolumetricTree *_tree,
                                     const Shape &_shape,
                                     typename PartitioningTree<Dimensions>::ShapeEnumerator _enumerator) noexcept;

        void EnsureCurrentRecordIsValid () noexcept;

        bool CheckIntersection (const void *_record) const noexcept;

        VolumetricTree *tree;
        std::size_t currentRecordIndex = 0u;
        Shape shape;
        typename PartitioningTree<Dimensions>::ShapeEnumerator shapeEnumerator;
    };

    static_assert (sizeof (ShapeIntersectionEnumerator) <= sizeof (std::uintptr_t) * 44u);

    VolumetricTree (const std::array<Dimension, Dimensions> &_dimensions) noexcept;

    VolumetricTree (const VolumetricTree &_other) = delete;

    VolumetricTree (VolumetricTree &&_other) noexcept = default;

    ~VolumetricTree () noexcept = default;

    [[nodiscard]] const std::array<Dimension, Dimensions> &GetDimensions () const noexcept;

    ShapeIntersectionEnumerator EnumerateIntersectingShapes (const Shape &_shape) noexcept;

    void Insert (const void *_record);

    void Update (const void *_record, const void *_backup) noexcept;

    bool IsPartitioningChanged (const void *_record, const void *_backup) const noexcept;

    void EraseWithBackup (const void *_record, const void *_backup) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (VolumetricTree);

private:
    static std::array<typename PartitioningTree<Dimensions>::Index, Dimensions> PreparePartitioningSpace (
        const std::array<Dimension, Dimensions> &_dimensions) noexcept;

    Shape ExtractShape (const void *_record) const noexcept;

    [[nodiscard]] typename PartitioningTree<Dimensions>::Shape ConvertToPartitioningShape (
        const Shape &_shape) const noexcept;

    std::array<Dimension, Dimensions> dimensions;
    PartitioningTree<Dimensions> partitioningTree;
};

using VolumetricTreeVariant = Container::Variant<VOLUMETRIC_TREE_VARIANTS ()>;

using ShapeIntersectionEnumeratorVariant = Container::Variant<VOLUMETRIC_TREE_VARIANTS (::ShapeIntersectionEnumerator)>;

class VolumetricIndex final : public IndexBase
{
public:
    using ValuePlaceholder = std::array<uint8_t, 8u>;

    struct DimensionDescriptor final
    {
        StandardLayout::FieldId minField;

        ValuePlaceholder min;

        StandardLayout::FieldId maxField;

        ValuePlaceholder max;
    };

    struct Dimension final
    {
        StandardLayout::Field minField;

        ValuePlaceholder min;

        StandardLayout::Field maxField;

        ValuePlaceholder max;
    };

    class DimensionIterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (DimensionIterator, const Dimension &);

    private:
        friend class VolumetricIndex;

        DimensionIterator (const VolumetricIndex *_index, std::size_t _dimensionIndex) noexcept;

        const VolumetricIndex *index = nullptr;
        std::size_t dimensionIndex = 0u;
    };

    class ShapeIntersectionReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (ShapeIntersectionReadCursor);

    private:
        friend class VolumetricIndex;

        ShapeIntersectionReadCursor (VolumetricIndex *_index,
                                     ShapeIntersectionEnumeratorVariant _baseEnumerator) noexcept;

        VolumetricIndex *index = nullptr;
        ShapeIntersectionEnumeratorVariant baseEnumerator;
    };

    class ShapeIntersectionEditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (ShapeIntersectionEditCursor);

    private:
        friend class VolumetricIndex;

        ShapeIntersectionEditCursor (VolumetricIndex *_index,
                                     ShapeIntersectionEnumeratorVariant _baseEnumerator) noexcept;

        template <typename Enumerator>
        void BeginRecordEdition (Enumerator &_enumerator) noexcept;

        template <typename Enumerator>
        bool EndRecordEdition (Enumerator &_enumerator) noexcept;

        VolumetricIndex *index = nullptr;
        ShapeIntersectionEnumeratorVariant baseEnumerator;
    };

    class RayIntersectionReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (RayIntersectionReadCursor);

    private:
        friend class VolumetricIndex;

        RayIntersectionReadCursor (VolumetricIndex *_index) noexcept;

        VolumetricIndex *index = nullptr;
    };

    class RayIntersectionEditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (RayIntersectionEditCursor);

    private:
        friend class VolumetricIndex;

        RayIntersectionEditCursor (VolumetricIndex *_index) noexcept;

        template <typename Enumerator>
        void BeginRecordEdition (Enumerator &_enumerator) noexcept;

        template <typename Enumerator>
        bool EndRecordEdition (Enumerator &_enumerator) noexcept;

        VolumetricIndex *index = nullptr;
    };

    VolumetricIndex (Storage *_storage,
                     const Container::Vector<VolumetricIndex::DimensionDescriptor> &_dimensions) noexcept;

    VolumetricIndex (const VolumetricIndex &_other) = delete;

    VolumetricIndex (VolumetricIndex &&_other) = delete;

    ~VolumetricIndex () noexcept = default;

    DimensionIterator BeginDimensions () const noexcept;

    DimensionIterator EndDimensions () const noexcept;

    ShapeIntersectionReadCursor LookupShapeIntersectionToRead (const void *_shape) noexcept;

    ShapeIntersectionEditCursor LookupShapeIntersectionToEdit (const void *_shape) noexcept;

    RayIntersectionReadCursor LookupRayIntersectionToRead (const void *_ray, float _rayLength) noexcept;

    RayIntersectionEditCursor LookupRayIntersectionToEdit (const void *_ray, float _rayLength) noexcept;

    void Drop () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (VolumetricIndex);

private:
    friend class Storage;

    static VolumetricTreeVariant CreateVolumetricTree (
        Storage *_storage, const Container::Vector<VolumetricIndex::DimensionDescriptor> &_dimensions) noexcept;

    void InsertRecord (const void *_record) noexcept;

    void OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    bool OnRecordChangedByMe (const void *_record, const void *_recordBackup) noexcept;

    void OnWriterClosed () noexcept;

    VolumetricTreeVariant tree;
    Container::Vector<const void *> reinsertionQueue {Memory::Profiler::AllocationGroup {"ReinsertionQueue"_us}};
};
} // namespace Emergence::Pegasus
