#include <algorithm>
#include <cassert>

#include <Pegasus/Storage.hpp>
#include <Pegasus/VolumetricIndex.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Pegasus
{
template <std::size_t Dimensions>
const Container::Vector<const void *> *PartitioningTree<Dimensions>::ShapeEnumerator::operator* () const noexcept
{
    if (!stack.Empty ())
    {
        return &stack.Back ().node->records;
    }

    return nullptr;
}

template <std::size_t Dimensions>
void PartitioningTree<Dimensions>::ShapeEnumerator::EraseRecord (std::size_t _index) noexcept
{
    assert (tree);
    assert (!stack.Empty ());
    assert (_index < stack.Back ().node->children.size ());
    Container::EraseExchangingWithLast (stack.Back ().node->records, stack.Back ().node->records.begin () + _index);

    while (stack.GetCount () > 1u && IsSafeToDelete (*stack.Back ().node))
    {
        stack.Back ().node->~Node ();
        tree->nodePool.Release (stack.Back ().node);
        stack.PopBack ();

        --stack.Back ().node->childrenCount;
        assert (stack.Back ().nextChildToVisit > 0u);
        stack.Back ().node->children[stack.Back ().nextChildToVisit - 1u] = nullptr;
    }
}

template <std::size_t Dimensions>
typename PartitioningTree<Dimensions>::ShapeEnumerator &
PartitioningTree<Dimensions>::ShapeEnumerator::operator++ () noexcept
{
    while (!stack.Empty ())
    {
        StackItem &top = stack.Back ();
        while (top.nextChildToVisit < NODE_CHILDREN_COUNT)
        {
            const Index toVisit = top.nextChildToVisit++;
            if (top.node->children[toVisit] && (toVisit & top.filterMask) == top.filterValue)
            {
                EnterNode (top.node->children[toVisit]);
                return *this;
            }
        }

        stack.PopBack ();
    }

    return *this;
}

template <std::size_t Dimensions>
PartitioningTree<Dimensions>::ShapeEnumerator::ShapeEnumerator (PartitioningTree *_tree, const Shape &_shape) noexcept
    : tree (_tree),
      shape (_shape)
{
    assert (tree);
    EnterNode (tree->root);
}

template <std::size_t Dimensions>
void PartitioningTree<Dimensions>::ShapeEnumerator::EnterNode (Node *_node) noexcept
{
    StackItem &item = stack.EmplaceBack ();
    item.node = _node;

    Index minMask = 0u;
    Index maxMask = 0u;

    for (Index index = 0u; index < Dimensions; ++index)
    {
        if (shape.bounds[index].min >= _node->center[index])
        {
            minMask |= 1u << index;
        }

        if (shape.bounds[index].max >= _node->center[index])
        {
            maxMask |= 1u << index;
        }
    }

    const Index difference = minMask ^ maxMask;
    const Index invertedDifference = ~difference;

    item.filterMask = invertedDifference;
    item.filterValue = minMask & invertedDifference;
}

template <std::size_t Dimensions>
PartitioningTree<Dimensions>::PartitioningTree (const std::array<Index, Dimensions> &_borders) noexcept
    : borders (_borders)
{
    maxLevel = std::numeric_limits<Index>::max ();
    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        // Borders shouldn't be too small.
        assert (std::countr_zero (borders[index]) > 2);
        maxLevel = std::min (maxLevel, static_cast<Index> (std::max (2u, std::countr_zero (borders[index]) - 2u)));
    }

    maxLevel = std::min (maxLevel, static_cast<Index> (Constants::VolumetricIndex::MAX_LEVELS));
    std::array<Index, Dimensions> center;

    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        assert (borders[index] > 1u && std::has_single_bit (borders[index]));
        center[index] = borders[index] / 2u;
    }

    root = new (nodePool.Acquire ()) Node (this, center);
}

template <std::size_t Dimensions>
PartitioningTree<Dimensions>::PartitioningTree (PartitioningTree &&_other) noexcept
    : borders (_other.borders),
      maxLevel (_other.maxLevel),
      nodePool (std::move (_other.nodePool)),
      root (_other.root)
{
    _other.root = nullptr;
}

template <std::size_t Dimensions>
PartitioningTree<Dimensions>::~PartitioningTree () noexcept
{
    if (root)
    {
        DeleteNodeWithChildren (root);
    }
}

template <std::size_t Dimensions>
const std::array<typename PartitioningTree<Dimensions>::Index, Dimensions> &PartitioningTree<Dimensions>::GetBorders ()
    const noexcept
{
    return borders;
}

template <std::size_t Dimensions>
typename PartitioningTree<Dimensions>::ShapeEnumerator PartitioningTree<Dimensions>::EnumerateIntersectingShapes (
    const Shape &_shape) noexcept
{
    return ShapeEnumerator (this, _shape);
}

template <std::size_t Dimensions>
void PartitioningTree<Dimensions>::Insert (const void *_record, const Shape &_shape)
{
    Node *current = root;
    std::size_t currentLevel = 0u;

    while (true)
    {
        std::size_t childIndex = SelectNodeChildForShape (*current, _shape);
        if (childIndex == SELECT_TOP_NODE)
        {
            current->records.emplace_back (_record);
            break;
        }

        if (!current->children[childIndex])
        {
            std::array<Index, Dimensions> center;
            const Index dividingShift = currentLevel + 2u;

            for (std::size_t index = 0u; index < Dimensions; ++index)
            {
                const Index halfSize = borders[index] >> dividingShift;
                assert (halfSize > 0u);

                if (childIndex & (1u << index))
                {
                    center[index] = current->center[index] + halfSize;
                }
                else
                {
                    center[index] = current->center[index] - halfSize;
                }
            }

            auto *newNode = new (nodePool.Acquire ()) Node {this, center};
            current->children[childIndex] = newNode;
            ++current->childrenCount;
        }

        current = current->children[childIndex];
        ++currentLevel;

        if (currentLevel == maxLevel - 1u)
        {
            current->records.emplace_back (_record);
            break;
        }
    }
}

template <std::size_t Dimensions>
void PartitioningTree<Dimensions>::Erase (const void *_record, const Shape &_shape) noexcept
{
    struct NodeIndexPair
    {
        Node *node = nullptr;
        std::size_t nodeIndexInParent = SELECT_TOP_NODE;
    };

    Container::InplaceVector<NodeIndexPair, Constants::VolumetricIndex::MAX_LEVELS> trace;
    trace.EmplaceBack () = {root, SELECT_TOP_NODE};

    Node *current = root;
    std::size_t currentLevel = 0u;

    auto tryRemoveRecord = [&current, _record] ()
    {
        auto iterator = std::find (current->records.begin (), current->records.end (), _record);
        // Otherwise tree integrity is broken: deterministic insertion algorithm should've put it here.
        assert (iterator != current->records.end ());
        Container::EraseExchangingWithLast (current->records, iterator);
    };

    while (true)
    {
        std::size_t childIndex = SelectNodeChildForShape (*current, _shape);
        if (childIndex == SELECT_TOP_NODE)
        {
            tryRemoveRecord ();
            break;
        }

        // Otherwise tree integrity is broken: deterministic insertion algorithm should've created this node.
        assert (current->children[childIndex]);
        current = current->children[childIndex];
        trace.EmplaceBack () = {current, childIndex};
        ++currentLevel;

        if (currentLevel == maxLevel - 1u)
        {
            tryRemoveRecord ();
            break;
        }
    }

    while (trace.Back ().node != root)
    {
        NodeIndexPair &last = trace.Back ();
        if (!IsSafeToDelete (*last.node))
        {
            break;
        }

        // No children, therefore we can delete directly.
        last.node->~Node ();
        nodePool.Release (last.node);
        trace.PopBack ();

        NodeIndexPair &newLast = trace.Back ();
        --newLast.node->childrenCount;
        newLast.node->children[last.nodeIndexInParent] = nullptr;
    }
}

template <std::size_t Dimensions>
PartitioningTree<Dimensions>::Node::Node (PartitioningTree *_tree,
                                          const std::array<Index, Dimensions> &_center) noexcept
    : records (_tree->nodePool.GetAllocationGroup ()),
      center (_center)
{
    for (std::size_t index = 0u; index < NODE_CHILDREN_COUNT; ++index)
    {
        children[index] = nullptr;
    }
}

template <std::size_t Dimensions>
PartitioningTree<Dimensions>::Node::~Node () noexcept
{
#ifndef NDEBUG
    // Ensure that all children are properly deleted by VolumetricTree::DeleteNode.
    for (std::size_t index = 0u; index < NODE_CHILDREN_COUNT; ++index)
    {
        assert (!children[index]);
    }
#endif
}

template <std::size_t Dimensions>
std::size_t PartitioningTree<Dimensions>::SelectNodeChildForShape (const Node &_node, const Shape &_shape) noexcept
{
    Index minNode = 0u;
    Index maxNode = 0u;

    for (Index index = 0u; index < Dimensions; ++index)
    {
        if (_shape.bounds[index].min >= _node.center[index])
        {
            minNode |= 1u << index;
        }

        if (_shape.bounds[index].max >= _node.center[index])
        {
            maxNode |= 1u << index;
        }
    }

    // If we cannot place shape in one node, we place it in the top node.
    return minNode == maxNode ? minNode : SELECT_TOP_NODE;
}

template <std::size_t Dimensions>
bool PartitioningTree<Dimensions>::IsSafeToDelete (const Node &_node) noexcept
{
    return _node.records.empty () && _node.childrenCount == 0u;
}

template <std::size_t Dimensions>
void PartitioningTree<Dimensions>::DeleteNodeWithChildren (Node *_node)
{
    if (!_node)
    {
        return;
    }

    for (std::size_t index = 0u; index < NODE_CHILDREN_COUNT; ++index)
    {
        DeleteNodeWithChildren (_node->children[index]);
        _node->children[index] = nullptr;
    }

    _node->~Node ();
    nodePool.Release (_node);
}

template <typename Unit, std::size_t Dimensions>
const void *VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator::operator* () const noexcept
{
    const Container::Vector<const void *> *recordsInNode = *shapeEnumerator;
    if (recordsInNode && currentRecordIndex < recordsInNode->size ())
    {
        return (*recordsInNode)[currentRecordIndex];
    }

    return nullptr;
}

template <typename Unit, std::size_t Dimensions>
typename VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator &
VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator::operator++ () noexcept
{
    const Container::Vector<const void *> *recordsInNode = *shapeEnumerator;
    assert (recordsInNode);

    while (true)
    {
        ++currentRecordIndex;
        while (currentRecordIndex >= recordsInNode->size ())
        {
            currentRecordIndex = 0u;
            ++shapeEnumerator;
            recordsInNode = *shapeEnumerator;

            if (!recordsInNode)
            {
                // We're done: whole tree is scanned.
                return *this;
            }
        }

        if (CheckIntersection ((*recordsInNode)[currentRecordIndex]))
        {
            // New intersection is found.
            return *this;
        }
    }
}

template <typename Unit, std::size_t Dimensions>
typename VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator &
VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator::operator~() noexcept
{
    const Container::Vector<const void *> *oldNode = *shapeEnumerator;
    assert (oldNode);
    shapeEnumerator.EraseRecord (currentRecordIndex);
    const Container::Vector<const void *> *newNode = *shapeEnumerator;

    if (oldNode != newNode)
    {
        currentRecordIndex = 0u;
    }

    EnsureCurrentRecordIsValid ();
    return *this;
}

template <typename Unit, std::size_t Dimensions>
VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator::ShapeIntersectionEnumerator (
    VolumetricTree *_tree,
    const VolumetricTree::Shape &_shape,
    typename PartitioningTree<Dimensions>::ShapeEnumerator _enumerator) noexcept
    : tree (_tree),
      shape (_shape),
      shapeEnumerator (std::move (_enumerator))
{
    EnsureCurrentRecordIsValid ();
}

template <typename Unit, std::size_t Dimensions>
void VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator::EnsureCurrentRecordIsValid () noexcept
{
    if (*shapeEnumerator)
    {
        const void *initialRecord = **this;
        // If no records in root or no intersection with first record in root: move ahead.
        if (!initialRecord || !CheckIntersection (initialRecord))
        {
            ++*this;
        }
    }
}

template <typename Unit, std::size_t Dimensions>
bool VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator::CheckIntersection (
    const void *_record) const noexcept
{
    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        const Unit min = *static_cast<const Unit *> (tree->dimensions[index].minField.GetValue (_record));
        const Unit max = *static_cast<const Unit *> (tree->dimensions[index].maxField.GetValue (_record));

        if (max < shape.bounds[index].min || min > shape.bounds[index].max)
        {
            return false;
        }
    }

    return true;
}

template <typename Unit, std::size_t Dimensions>
VolumetricTree<Unit, Dimensions>::VolumetricTree (const std::array<Dimension, Dimensions> &_dimensions) noexcept
    : dimensions (_dimensions),
      partitioningTree (PreparePartitioningSpace (_dimensions))
{
}

template <typename Unit, std::size_t Dimensions>
const std::array<typename VolumetricTree<Unit, Dimensions>::Dimension, Dimensions>
    &VolumetricTree<Unit, Dimensions>::GetDimensions () const noexcept
{
    return dimensions;
}

template <typename Unit, std::size_t Dimensions>
typename VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator
VolumetricTree<Unit, Dimensions>::EnumerateIntersectingShapes (const VolumetricTree::Shape &_shape) noexcept
{
    return {this, _shape, partitioningTree.EnumerateIntersectingShapes (ConvertToPartitioningShape (_shape))};
}

template <typename Unit, std::size_t Dimensions>
void VolumetricTree<Unit, Dimensions>::Insert (const void *_record)
{
    partitioningTree.Insert (_record, ConvertToPartitioningShape (ExtractShape (_record)));
}

template <typename Unit, std::size_t Dimensions>
void VolumetricTree<Unit, Dimensions>::Update (const void *_record, const void *_backup) noexcept
{
    typename PartitioningTree<Dimensions>::Shape oldShape = ConvertToPartitioningShape (ExtractShape (_backup));
    typename PartitioningTree<Dimensions>::Shape newShape = ConvertToPartitioningShape (ExtractShape (_record));

    if (oldShape != newShape)
    {
        // Right now we're not expecting huge objects to move, therefore this approach is good enough.
        partitioningTree.Erase (_record, oldShape);
        partitioningTree.Insert (_record, newShape);
    }
}

template <typename Unit, std::size_t Dimensions>
bool VolumetricTree<Unit, Dimensions>::IsPartitioningChanged (const void *_record, const void *_backup) const noexcept
{
    typename PartitioningTree<Dimensions>::Shape oldShape = ConvertToPartitioningShape (ExtractShape (_backup));
    typename PartitioningTree<Dimensions>::Shape newShape = ConvertToPartitioningShape (ExtractShape (_record));
    return oldShape != newShape;
}

template <typename Unit, std::size_t Dimensions>
void VolumetricTree<Unit, Dimensions>::EraseWithBackup (const void *_record, const void *_backup) noexcept
{
    partitioningTree.Erase (_record, ConvertToPartitioningShape (ExtractShape (_backup)));
}

template <typename Unit, std::size_t Dimensions>
std::array<typename PartitioningTree<Dimensions>::Index, Dimensions>
VolumetricTree<Unit, Dimensions>::PreparePartitioningSpace (
    const std::array<Dimension, Dimensions> &_dimensions) noexcept
{
    std::array<typename PartitioningTree<Dimensions>::Index, Dimensions> result;
    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        const Unit space = _dimensions[index].maxBorder - _dimensions[index].minBorder;
        const float floatingPartitions =
            static_cast<float> (space) * Constants::VolumetricIndex::IDEAL_UNIT_TO_PARTITION_SCALE;

        const auto roundedPartitions =
            static_cast<typename PartitioningTree<Dimensions>::Index> (std::ceilf (floatingPartitions));

        constexpr std::size_t BIT_COUNT = sizeof (typename PartitioningTree<Dimensions>::Index) * 8u;
        result[index] = 1u << (BIT_COUNT - std::countl_zero (roundedPartitions));
    }

    return result;
}

template <typename Unit, std::size_t Dimensions>
typename VolumetricTree<Unit, Dimensions>::Shape VolumetricTree<Unit, Dimensions>::ExtractShape (
    const void *_record) const noexcept
{
    Shape shape;
    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        shape.bounds[index].min = *static_cast<const Unit *> (dimensions[index].minField.GetValue (_record));
        shape.bounds[index].max = *static_cast<const Unit *> (dimensions[index].maxField.GetValue (_record));
    }

    return shape;
}

template <typename Unit, std::size_t Dimensions>
typename PartitioningTree<Dimensions>::Shape VolumetricTree<Unit, Dimensions>::ConvertToPartitioningShape (
    const VolumetricTree::Shape &_shape) const noexcept
{
    typename PartitioningTree<Dimensions>::Shape convertedShape;
    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        const typename PartitioningTree<Dimensions>::Index partitions = partitioningTree.GetBorders ()[index];
        const Unit space = dimensions[index].maxBorder - dimensions[index].minBorder;

        const Unit localizedMinValue = _shape.bounds[index].min - dimensions[index].minBorder;
        const Unit localizedMaxValue = _shape.bounds[index].max - dimensions[index].minBorder;

        const float minPercent = static_cast<float> (localizedMinValue) / static_cast<float> (space);
        const float maxPercent = static_cast<float> (localizedMaxValue) / static_cast<float> (space);

        convertedShape.bounds[index].min = static_cast<typename PartitioningTree<Dimensions>::Index> (
            std::floorf (minPercent * static_cast<float> (partitions)));
        convertedShape.bounds[index].max = static_cast<typename PartitioningTree<Dimensions>::Index> (
            std::ceilf (maxPercent * static_cast<float> (partitions)));
    }

    return convertedShape;
}

VolumetricIndex::DimensionIterator::DimensionIterator (const VolumetricIndex::DimensionIterator &_other) noexcept =
    default;

VolumetricIndex::DimensionIterator::DimensionIterator (VolumetricIndex::DimensionIterator &&_other) noexcept = default;

VolumetricIndex::DimensionIterator::~DimensionIterator () noexcept = default;

VolumetricIndex::DimensionIterator &VolumetricIndex::DimensionIterator::operator++ () noexcept
{
    ++dimensionIndex;
    return *this;
}

VolumetricIndex::DimensionIterator VolumetricIndex::DimensionIterator::operator++ (int) noexcept
{
    DimensionIterator previous = *this;
    ++*this;
    return previous;
}

VolumetricIndex::DimensionIterator &VolumetricIndex::DimensionIterator::operator-- () noexcept
{
    --dimensionIndex;
    return *this;
}

VolumetricIndex::DimensionIterator VolumetricIndex::DimensionIterator::operator-- (int) noexcept
{
    DimensionIterator previous = *this;
    --*this;
    return previous;
}

const VolumetricIndex::Dimension &VolumetricIndex::DimensionIterator::operator* () const noexcept
{
    return std::visit (
        [this] (const auto &_tree) -> const Dimension &
        {
            const auto &sourceDimension = _tree.GetDimensions ()[dimensionIndex];
            return *reinterpret_cast<const Dimension *> (&sourceDimension);
        },
        index->tree);
}

bool VolumetricIndex::DimensionIterator::operator== (const VolumetricIndex::DimensionIterator &_other) const noexcept
{
    assert (index == _other.index);
    return dimensionIndex == _other.dimensionIndex;
}

bool VolumetricIndex::DimensionIterator::operator!= (const VolumetricIndex::DimensionIterator &_other) const noexcept
{
    return !(*this == _other);
}

VolumetricIndex::DimensionIterator &VolumetricIndex::DimensionIterator::operator= (
    const VolumetricIndex::DimensionIterator &_other) noexcept = default;

VolumetricIndex::DimensionIterator &VolumetricIndex::DimensionIterator::operator= (
    VolumetricIndex::DimensionIterator &&_other) noexcept = default;

VolumetricIndex::DimensionIterator::DimensionIterator (const VolumetricIndex *_index,
                                                       std::size_t _dimensionIndex) noexcept
    : index (_index),
      dimensionIndex (_dimensionIndex)
{
}

VolumetricIndex::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    const VolumetricIndex::ShapeIntersectionReadCursor &_other) noexcept
    : index (_other.index),
      baseEnumerator (_other.baseEnumerator)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

VolumetricIndex::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    VolumetricIndex::ShapeIntersectionReadCursor &&_other) noexcept
    : index (_other.index),
      baseEnumerator (std::move (_other.baseEnumerator))
{
    assert (index);
    _other.index = nullptr;
}

VolumetricIndex::ShapeIntersectionReadCursor::~ShapeIntersectionReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

const void *VolumetricIndex::ShapeIntersectionReadCursor::operator* () const noexcept
{
    return std::visit (
        [] (auto &_enumerator)
        {
            return *_enumerator;
        },
        baseEnumerator);
}

VolumetricIndex::ShapeIntersectionReadCursor &VolumetricIndex::ShapeIntersectionReadCursor::operator++ () noexcept
{
    std::visit (
        [] (auto &_enumerator)
        {
            ++_enumerator;
        },
        baseEnumerator);
    return *this;
}

VolumetricIndex::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    VolumetricIndex *_index, ShapeIntersectionEnumeratorVariant _baseEnumerator) noexcept
    : index (_index),
      baseEnumerator (std::move (_baseEnumerator))
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

VolumetricIndex::ShapeIntersectionEditCursor::ShapeIntersectionEditCursor (
    VolumetricIndex::ShapeIntersectionEditCursor &&_other) noexcept
    : index (_other.index),
      baseEnumerator (std::move (_other.baseEnumerator))
{
    assert (index);
    _other.index = nullptr;
}

VolumetricIndex::ShapeIntersectionEditCursor::~ShapeIntersectionEditCursor () noexcept
{
    if (index)
    {
        std::visit (
            [this] (auto &_enumerator)
            {
                EndRecordEdition (_enumerator);
            },
            baseEnumerator);

        --index->activeCursors;
        index->storage->UnregisterWriter ();
    }
}

void *VolumetricIndex::ShapeIntersectionEditCursor::operator* () noexcept
{
    const void *record = std::visit (
        [] (const auto &_enumerator)
        {
            return *_enumerator;
        },
        baseEnumerator);
    return const_cast<void *> (record);
}

VolumetricIndex::ShapeIntersectionEditCursor &VolumetricIndex::ShapeIntersectionEditCursor::operator++ () noexcept
{
    std::visit (
        [this] (auto &_enumerator)
        {
            if (!EndRecordEdition (_enumerator))
            {
                ++_enumerator;
            }

            BeginRecordEdition (_enumerator);
        },
        baseEnumerator);
    return *this;
}

VolumetricIndex::ShapeIntersectionEditCursor &VolumetricIndex::ShapeIntersectionEditCursor::operator~() noexcept
{
    std::visit (
        [this] (auto &_enumerator)
        {
            const void *record = *_enumerator;
            ~_enumerator;
            index->storage->DeleteRecord (const_cast<void *> (record), index);
            BeginRecordEdition (_enumerator);
        },
        baseEnumerator);
    return *this;
}

VolumetricIndex::ShapeIntersectionEditCursor::ShapeIntersectionEditCursor (
    VolumetricIndex *_index, ShapeIntersectionEnumeratorVariant _baseEnumerator) noexcept
    : index (_index),
      baseEnumerator (std::move (_baseEnumerator))
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterWriter ();

    std::visit (
        [this] (auto &_enumerator)
        {
            BeginRecordEdition (_enumerator);
        },
        baseEnumerator);
}

template <typename Enumerator>
void VolumetricIndex::ShapeIntersectionEditCursor::BeginRecordEdition (Enumerator &_enumerator) noexcept
{
    if (const void *record = *_enumerator)
    {
        index->storage->BeginRecordEdition (record);
    }
}

template <typename Enumerator>
bool VolumetricIndex::ShapeIntersectionEditCursor::EndRecordEdition (Enumerator &_enumerator) noexcept
{
    if (const void *record = *_enumerator)
    {
        if (index->storage->EndRecordEdition (record, index) &&
            index->OnRecordChangedByMe (record, index->storage->GetEditedRecordBackup ()))
        {
            ~_enumerator;
            return true;
        }
    }

    return false;
}

VolumetricIndex::RayIntersectionReadCursor::RayIntersectionReadCursor (
    const VolumetricIndex::RayIntersectionReadCursor &_other) noexcept
    : index (_other.index)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

VolumetricIndex::RayIntersectionReadCursor::RayIntersectionReadCursor (
    VolumetricIndex::RayIntersectionReadCursor &&_other) noexcept
    : index (_other.index)
{
    assert (index);
    _other.index = nullptr;
}

VolumetricIndex::RayIntersectionReadCursor::~RayIntersectionReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

const void *VolumetricIndex::RayIntersectionReadCursor::operator* () const noexcept
{
    // TODO: Implement.
    return nullptr;
}

VolumetricIndex::RayIntersectionReadCursor &VolumetricIndex::RayIntersectionReadCursor::operator++ () noexcept
{
    // TODO: Implement.
    return *this;
}

VolumetricIndex::RayIntersectionReadCursor::RayIntersectionReadCursor (VolumetricIndex *_index) noexcept
    : index (_index)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

VolumetricIndex::RayIntersectionEditCursor::RayIntersectionEditCursor (
    VolumetricIndex::RayIntersectionEditCursor &&_other) noexcept
    : index (_other.index)
{
    assert (index);
    _other.index = nullptr;
}

VolumetricIndex::RayIntersectionEditCursor::~RayIntersectionEditCursor () noexcept
{
    if (index)
    {
        // TODO: End record edition.
        --index->activeCursors;
        index->storage->UnregisterWriter ();
    }
}

void *VolumetricIndex::RayIntersectionEditCursor::operator* () noexcept
{
    // TODO: Implement.
    return nullptr;
}

VolumetricIndex::RayIntersectionEditCursor &VolumetricIndex::RayIntersectionEditCursor::operator++ () noexcept
{
    // TODO: Implement.
    return *this;
}

VolumetricIndex::RayIntersectionEditCursor &VolumetricIndex::RayIntersectionEditCursor::operator~() noexcept
{
    // TODO: Implement.
    return *this;
}

VolumetricIndex::RayIntersectionEditCursor::RayIntersectionEditCursor (VolumetricIndex *_index) noexcept
    : index (_index)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterWriter ();
}

template <typename Enumerator>
void VolumetricIndex::RayIntersectionEditCursor::BeginRecordEdition (Enumerator &_enumerator) noexcept
{
    // TODO: Implement.
}

template <typename Enumerator>
bool VolumetricIndex::RayIntersectionEditCursor::EndRecordEdition (Enumerator &_enumerator) noexcept
{
    // TODO: Implement.
    return false;
}

VolumetricIndex::VolumetricIndex (Storage *_storage,
                                  const Container::Vector<VolumetricIndex::DimensionDescriptor> &_dimensions) noexcept
    : IndexBase (_storage),
      tree (CreateVolumetricTree (_storage, _dimensions))
{
}

VolumetricIndex::DimensionIterator VolumetricIndex::BeginDimensions () const noexcept
{
    return {this, 0u};
}

VolumetricIndex::DimensionIterator VolumetricIndex::EndDimensions () const noexcept
{
    std::size_t dimensionCount = std::visit (
        [] (const auto &_tree)
        {
            return _tree.GetDimensions ().size ();
        },
        tree);
    return {this, dimensionCount};
}

VolumetricIndex::ShapeIntersectionReadCursor VolumetricIndex::LookupShapeIntersectionToRead (
    const void *_shape) noexcept
{
    ShapeIntersectionEnumeratorVariant enumerator = std::visit (
        [_shape] (auto &_tree) -> ShapeIntersectionEnumeratorVariant
        {
            using Tree = std::decay_t<decltype (_tree)>;
            return _tree.EnumerateIntersectingShapes (*static_cast<const typename Tree::Shape *> (_shape));
        },
        tree);
    return {this, std::move (enumerator)};
}

VolumetricIndex::ShapeIntersectionEditCursor VolumetricIndex::LookupShapeIntersectionToEdit (
    const void *_shape) noexcept
{
    ShapeIntersectionEnumeratorVariant enumerator = std::visit (
        [_shape] (auto &_tree) -> ShapeIntersectionEnumeratorVariant
        {
            using Tree = std::decay_t<decltype (_tree)>;
            return _tree.EnumerateIntersectingShapes (*static_cast<const typename Tree::Shape *> (_shape));
        },
        tree);
    return {this, std::move (enumerator)};
}

VolumetricIndex::RayIntersectionReadCursor VolumetricIndex::LookupRayIntersectionToRead (
    [[maybe_unused]] const void *_ray, [[maybe_unused]] float _rayLength) noexcept
{
    return {this};
}

VolumetricIndex::RayIntersectionEditCursor VolumetricIndex::LookupRayIntersectionToEdit (
    [[maybe_unused]] const void *_ray, [[maybe_unused]] float _rayLength) noexcept
{
    return {this};
}

void VolumetricIndex::Drop () noexcept
{
    assert (CanBeDropped ());
    assert (storage);
    storage->DropIndex (*this);
}

template <typename Unit, std::size_t Count>
std::array<typename VolumetricTree<Unit, Count>::Dimension, Count> ConvertDimensions (
    Storage *_storage, const Container::Vector<VolumetricIndex::DimensionDescriptor> &_dimensions)
{
    assert (_dimensions.size () == Count);
    std::array<typename VolumetricTree<Unit, Count>::Dimension, Count> result;

    for (std::size_t index = 0u; index < Count; ++index)
    {
        result[index] = {_storage->GetRecordMapping ().GetField (_dimensions[index].minField),
                         block_cast<Unit> (_dimensions[index].min),
                         _storage->GetRecordMapping ().GetField (_dimensions[index].maxField),
                         block_cast<Unit> (_dimensions[index].max)};
    }

    return result;
}

VolumetricTreeVariant VolumetricIndex::CreateVolumetricTree (
    Storage *_storage, const Container::Vector<VolumetricIndex::DimensionDescriptor> &_dimensions) noexcept
{
    static_assert (Constants::VolumetricIndex::MAX_DIMENSIONS <= 3u);
    assert (!_dimensions.empty ());
    assert (_dimensions.size () <= Constants::VolumetricIndex::MAX_DIMENSIONS);

    StandardLayout::Field selectionBaseField = _storage->GetRecordMapping ().GetField (_dimensions.front ().minField);
    assert (selectionBaseField.GetArchetype () == StandardLayout::FieldArchetype::INT ||
            selectionBaseField.GetArchetype () == StandardLayout::FieldArchetype::UINT ||
            selectionBaseField.GetArchetype () == StandardLayout::FieldArchetype::FLOAT);

#define UNHANDLED_COMBINATION                                                                                          \
    assert (false);                                                                                                    \
    return VolumetricTree<uint8_t, 1u> ({})

#define HANDLE_DIMENSION_COUNT(Count)                                                                                  \
    case Count:                                                                                                        \
        switch (selectionBaseField.GetArchetype ())                                                                    \
        {                                                                                                              \
        case StandardLayout::FieldArchetype::INT:                                                                      \
            switch (selectionBaseField.GetSize ())                                                                     \
            {                                                                                                          \
            case 1u:                                                                                                   \
                return VolumetricTree<int8_t, Count> (ConvertDimensions<int8_t, Count> (_storage, _dimensions));       \
            case 2u:                                                                                                   \
                return VolumetricTree<int16_t, Count> (ConvertDimensions<int16_t, Count> (_storage, _dimensions));     \
            case 4u:                                                                                                   \
                return VolumetricTree<int32_t, Count> (ConvertDimensions<int32_t, Count> (_storage, _dimensions));     \
            case 8u:                                                                                                   \
                return VolumetricTree<int64_t, Count> (ConvertDimensions<int64_t, Count> (_storage, _dimensions));     \
            }                                                                                                          \
                                                                                                                       \
        case StandardLayout::FieldArchetype::UINT:                                                                     \
            switch (selectionBaseField.GetSize ())                                                                     \
            {                                                                                                          \
            case 1u:                                                                                                   \
                return VolumetricTree<uint8_t, Count> (ConvertDimensions<uint8_t, Count> (_storage, _dimensions));     \
            case 2u:                                                                                                   \
                return VolumetricTree<uint16_t, Count> (ConvertDimensions<uint16_t, Count> (_storage, _dimensions));   \
            case 4u:                                                                                                   \
                return VolumetricTree<uint32_t, Count> (ConvertDimensions<uint32_t, Count> (_storage, _dimensions));   \
            case 8u:                                                                                                   \
                return VolumetricTree<uint64_t, Count> (ConvertDimensions<uint64_t, Count> (_storage, _dimensions));   \
            }                                                                                                          \
                                                                                                                       \
        case StandardLayout::FieldArchetype::FLOAT:                                                                    \
            switch (selectionBaseField.GetSize ())                                                                     \
            {                                                                                                          \
            case 4u:                                                                                                   \
                return VolumetricTree<float, Count> (ConvertDimensions<float, Count> (_storage, _dimensions));         \
            case 8u:                                                                                                   \
                return VolumetricTree<double, Count> (ConvertDimensions<double, Count> (_storage, _dimensions));       \
            }                                                                                                          \
                                                                                                                       \
        case StandardLayout::FieldArchetype::BIT:                                                                      \
        case StandardLayout::FieldArchetype::STRING:                                                                   \
        case StandardLayout::FieldArchetype::BLOCK:                                                                    \
        case StandardLayout::FieldArchetype::UNIQUE_STRING:                                                            \
        case StandardLayout::FieldArchetype::NESTED_OBJECT:                                                            \
            UNHANDLED_COMBINATION;                                                                                     \
        }

    switch (_dimensions.size ())
    {
        HANDLE_DIMENSION_COUNT (1u)
        HANDLE_DIMENSION_COUNT (2u)
        HANDLE_DIMENSION_COUNT (3u)
    }

    UNHANDLED_COMBINATION;
}

void VolumetricIndex::InsertRecord (const void *_record) noexcept
{
    std::visit (
        [_record] (auto &_tree)
        {
            _tree.Insert (_record);
        },
        tree);
}

void VolumetricIndex::OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept
{
    std::visit (
        [_record, _recordBackup] (auto &_tree)
        {
            _tree.EraseWithBackup (_record, _recordBackup);
        },
        tree);
}

void VolumetricIndex::OnRecordChanged (const void *_record, const void *_recordBackup) noexcept
{
    std::visit (
        [_record, _recordBackup] (auto &_tree)
        {
            _tree.Update (_record, _recordBackup);
        },
        tree);
}

bool VolumetricIndex::OnRecordChangedByMe (const void *_record, const void *_recordBackup) noexcept
{
    return std::visit (
        [this, _record, _recordBackup] (auto &_tree)
        {
            if (_tree.IsPartitioningChanged (_record, _recordBackup))
            {
                reinsertionQueue.emplace_back (_record);
                // Return true to inform cursor that record shall be deleted.
                return true;
            }

            // No need to update the tree.
            return false;
        },
        tree);
}

void VolumetricIndex::OnWriterClosed () noexcept
{
    std::visit (
        [this] (auto &_tree)
        {
            for (const void *record : reinsertionQueue)
            {
                _tree.Insert (record);
            }

            reinsertionQueue.clear ();
        },
        tree);
}
} // namespace Emergence::Pegasus
