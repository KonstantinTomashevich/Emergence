#include <algorithm>
#include <cmath>
#include <limits>

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
    EMERGENCE_ASSERT (tree);
    EMERGENCE_ASSERT (!stack.Empty ());
    EMERGENCE_ASSERT (_index < stack.Back ().node->children.size ());
    Container::EraseExchangingWithLast (stack.Back ().node->records, stack.Back ().node->records.begin () + _index);
    bool needToMove = false;

    while (stack.GetCount () > 1u && IsSafeToDelete (*stack.Back ().node))
    {
        needToMove = true;
        stack.Back ().node->~Node ();
        tree->nodePool.Release (stack.Back ().node);
        stack.PopBack ();

        --stack.Back ().node->childrenCount;
        EMERGENCE_ASSERT (stack.Back ().nextChildToVisit > 0u);
        stack.Back ().node->children[stack.Back ().nextChildToVisit - 1u] = nullptr;
    }

    if (needToMove)
    {
        ++*this;
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
    EMERGENCE_ASSERT (tree);
    EnterNode (tree->root);
}

template <std::size_t Dimensions>
void PartitioningTree<Dimensions>::ShapeEnumerator::EnterNode (Node *_node) noexcept
{
    StackItem &item = stack.EmplaceBack ();
    item.node = _node;

    Index minMask = 0u;
    Index maxMask = 0u;

    for (std::size_t index = 0u; index < Dimensions; ++index)
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
void PartitioningTree<Dimensions>::RayEnumerator::EraseRecord (std::size_t _index) noexcept
{
    EMERGENCE_ASSERT (tree);
    EMERGENCE_ASSERT (!stack.Empty ());
    EMERGENCE_ASSERT (_index < stack.Back ()->children.size ());
    Container::EraseExchangingWithLast (stack.Back ()->records, stack.Back ()->records.begin () + _index);
    bool needToMove = false;

    while (stack.GetCount () > 1u && IsSafeToDelete (*stack.Back ()))
    {
        needToMove = true;
        stack.Back ()->~Node ();
        tree->nodePool.Release (stack.Back ());
        stack.PopBack ();

        --stack.Back ()->childrenCount;
        const Index nextChildIndex = GetNextChildIndex ();
        EMERGENCE_ASSERT (nextChildIndex < NODE_CHILDREN_COUNT);
        stack.Back ()->children[nextChildIndex] = nullptr;
    }

    if (needToMove)
    {
        ++*this;
    }
}

template <std::size_t Dimensions>
const Container::Vector<const void *> *PartitioningTree<Dimensions>::RayEnumerator::operator* () const noexcept
{
    if (!stack.Empty ())
    {
        return &stack.Back ()->records;
    }

    return nullptr;
}

template <std::size_t Dimensions>
typename PartitioningTree<Dimensions>::RayEnumerator &
PartitioningTree<Dimensions>::RayEnumerator::operator++ () noexcept
{
    while (!stack.Empty ())
    {
        if (ContinueDescentToTarget ())
        {
            break;
        }

        MoveToNextTarget ();
    }

    return *this;
}

template <std::size_t Dimensions>
PartitioningTree<Dimensions>::RayEnumerator::RayEnumerator (
    PartitioningTree *_tree,
    const PartitioningTree::Ray &_ray,
    float _maxDistance,
    const std::array<float, Dimensions> &_distanceFactors) noexcept
    : tree (_tree),
      maxDistanceToTravelSquared (_maxDistance * _maxDistance),
      distanceFactors (_distanceFactors)
{
    float directionSquareSum = 0.0f;
    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        currentPoint.coordinates[dimension] = _ray.axis[dimension].origin;
        const float offset = _ray.axis[dimension].direction > 0.0f ? Constants::VolumetricIndex::EPSILON :
                                                                     -Constants::VolumetricIndex::EPSILON;
        currentTargetNode[dimension] = static_cast<Index> (floorf (currentPoint.coordinates[dimension] + offset));

        normalizedDirection.coordinates[dimension] = _ray.axis[dimension].direction;
        directionSquareSum += normalizedDirection.coordinates[dimension] * normalizedDirection.coordinates[dimension];
    }

    const float directionLength = sqrtf (directionSquareSum);
    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        normalizedDirection.coordinates[dimension] /= directionLength;
    }

    stack.EmplaceBack (tree->root);
}

template <std::size_t Dimensions>
typename PartitioningTree<Dimensions>::Index PartitioningTree<Dimensions>::RayEnumerator::GetNextChildIndex ()
    const noexcept
{
    Index nextChildMask = tree->border >> stack.GetCount ();
    if (nextChildMask == 0u)
    {
        return NODE_CHILDREN_COUNT;
    }

    Index nextChildIndex = 0u;
    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        if (currentTargetNode[dimension] & nextChildMask)
        {
            nextChildIndex |= 1u << dimension;
        }
    }

    return nextChildIndex;
}

template <std::size_t Dimensions>
bool PartitioningTree<Dimensions>::RayEnumerator::ContinueDescentToTarget () noexcept
{
    const Index nextChildIndex = GetNextChildIndex ();
    if (nextChildIndex >= NODE_CHILDREN_COUNT)
    {
        return false;
    }

    Node *top = stack.Back ();
    if (Node *child = top->children[nextChildIndex])
    {
        stack.EmplaceBack (child);
        return true;
    }

    return false;
}

template <std::size_t Dimensions>
void PartitioningTree<Dimensions>::RayEnumerator::MoveToNextTarget () noexcept
{
    struct DirectionInfo
    {
        bool negative;
        bool positive;
        float offset;
    };

    std::array<DirectionInfo, Dimensions> directionInfo;
    const Index topLevelChildSize = tree->border >> stack.GetCount ();
    const Index topLevelMask = ~(topLevelChildSize - 1u);

    Index movementDimension = 0u;
    float smallestMoveLength = std::numeric_limits<float>::max ();

    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        directionInfo[dimension].negative =
            normalizedDirection.coordinates[dimension] < -Emergence::Pegasus::Constants::VolumetricIndex::EPSILON;
        directionInfo[dimension].positive =
            normalizedDirection.coordinates[dimension] > Emergence::Pegasus::Constants::VolumetricIndex::EPSILON;

        if (directionInfo[dimension].negative || directionInfo[dimension].positive)
        {
            directionInfo[dimension].offset = directionInfo[dimension].positive ?
                                                  Emergence::Pegasus::Constants::VolumetricIndex::EPSILON :
                                                  -Emergence::Pegasus::Constants::VolumetricIndex::EPSILON;

            const auto floored =
                static_cast<Index> (floorf (currentPoint.coordinates[dimension] + directionInfo[dimension].offset));
            const Index topLevelCoordinate = floored & topLevelMask;

            const float moveTarget = directionInfo[dimension].positive ?
                                         static_cast<float> (topLevelCoordinate + topLevelChildSize) :
                                         (static_cast<float> (topLevelCoordinate) - 1e-5f);

            const float moveDistance = moveTarget - currentPoint.coordinates[dimension];
            const float moveLength = moveDistance / normalizedDirection.coordinates[dimension];

            if (moveLength > 0.0f && moveLength < smallestMoveLength)
            {
                movementDimension = static_cast<Index> (dimension);
                smallestMoveLength = moveLength;
            }
        }
    }

    float distanceTravelledSquared = 0.0f;
    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        if (directionInfo[dimension].negative || directionInfo[dimension].positive)
        {
            const float perAxis = normalizedDirection.coordinates[dimension] * smallestMoveLength;
            distanceTravelledSquared = distanceFactors[dimension] * perAxis * perAxis;
        }
    }

    traveledDistanceSquared += distanceTravelledSquared;
    if (traveledDistanceSquared > maxDistanceToTravelSquared)
    {
        // We've travelled maximum distance: it's time to stop.
        Stop ();
        return;
    }

    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        if (directionInfo[dimension].negative || directionInfo[dimension].positive)
        {
            currentPoint.coordinates[dimension] += normalizedDirection.coordinates[dimension] * smallestMoveLength;
            const float pointWithOffset = currentPoint.coordinates[dimension] + directionInfo[dimension].offset;

            if (pointWithOffset < 0.0f || pointWithOffset >= static_cast<float> (tree->border))
            {
                Stop ();
                return;
            }

            currentTargetNode[dimension] = static_cast<Index> (floorf (pointWithOffset));
        }
    }

    const std::size_t levelsCrossed =
        std::countr_zero (directionInfo[movementDimension].positive ? currentTargetNode[movementDimension] :
                                                                      currentTargetNode[movementDimension] + 1u);

    // Last level is not supported here, because it is never really created, as it has no representable center.
    EMERGENCE_ASSERT (levelsCrossed > 0u);

    const std::size_t maxLevelInStack = std::countr_zero (tree->border) - levelsCrossed;
    if (stack.GetCount () > maxLevelInStack)
    {
        stack.DropTrailing (stack.GetCount () - maxLevelInStack);
    }
}

template <std::size_t Dimensions>
void PartitioningTree<Dimensions>::RayEnumerator::Stop () noexcept
{
    stack.Clear ();
}

template <std::size_t Dimensions>
PartitioningTree<Dimensions>::PartitioningTree (Index _border) noexcept
    : border (_border)
{
    EMERGENCE_ASSERT (border > 1u);
    EMERGENCE_ASSERT (std::has_single_bit (border));
    EMERGENCE_ASSERT (std::countr_zero (border) > 2);

    maxLevel = std::min (static_cast<Index> (std::max (2u, std::countr_zero (border) - 2u)),
                         static_cast<Index> (Constants::VolumetricIndex::MAX_LEVELS));

    std::array<Index, Dimensions> center;

    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        center[index] = border / 2u;
    }

    root = new (nodePool.Acquire ()) Node (this, center);
}

template <std::size_t Dimensions>
PartitioningTree<Dimensions>::PartitioningTree (PartitioningTree &&_other) noexcept
    : border (_other.border),
      maxLevel (_other.maxLevel),
      nodePool (std::move (_other.nodePool)),
      root (_other.root)
{
    _other.root = nullptr;
}

template <std::size_t Dimensions>
typename PartitioningTree<Dimensions>::Index PartitioningTree<Dimensions>::GetBorder () const noexcept
{
    return border;
}

template <std::size_t Dimensions>
typename PartitioningTree<Dimensions>::ShapeEnumerator PartitioningTree<Dimensions>::EnumerateIntersectingShapes (
    const Shape &_shape) noexcept
{
    return {this, _shape};
}

template <std::size_t Dimensions>
typename PartitioningTree<Dimensions>::RayEnumerator PartitioningTree<Dimensions>::EnumerateIntersectingShapes (
    const Ray &_ray, float _maxDistance, const std::array<float, Dimensions> &_distanceFactors) noexcept
{
    return {this, _ray, _maxDistance, _distanceFactors};
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
            const auto dividingShift = static_cast<Index> (currentLevel + 2u);
            const Index halfSize = border >> dividingShift;
            EMERGENCE_ASSERT (halfSize > 0u);

            for (std::size_t index = 0u; index < Dimensions; ++index)
            {
                if (childIndex & (std::size_t {1u} << index))
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
        EMERGENCE_ASSERT (iterator != current->records.end ());
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
        EMERGENCE_ASSERT (current->children[childIndex]);
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
void PartitioningTree<Dimensions>::Clear () noexcept
{
    for (Node *&child : root->children)
    {
        DeleteNodeWithChildren (child);
        child = nullptr;
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
std::size_t PartitioningTree<Dimensions>::SelectNodeChildForShape (const Node &_node, const Shape &_shape) noexcept
{
    Index minNode = 0u;
    Index maxNode = 0u;

    for (std::size_t index = 0u; index < Dimensions; ++index)
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

template <typename Unit, std::size_t Dimensions>
template <typename Enumerator, typename Geometry, typename Inheritor>
const void *VolumetricTree<Unit, Dimensions>::EnumeratorWrapper<Enumerator, Geometry, Inheritor>::operator* ()
    const noexcept
{
    const Container::Vector<const void *> *recordsInNode = *enumerator;
    if (recordsInNode && currentRecordIndex < recordsInNode->size ())
    {
        return (*recordsInNode)[currentRecordIndex];
    }

    return nullptr;
}

template <typename Unit, std::size_t Dimensions>
template <typename Enumerator, typename Geometry, typename Inheritor>
Inheritor &VolumetricTree<Unit, Dimensions>::EnumeratorWrapper<Enumerator, Geometry, Inheritor>::operator++ () noexcept
{
    const Container::Vector<const void *> *recordsInNode = *enumerator;
    EMERGENCE_ASSERT (recordsInNode);

    while (true)
    {
        ++currentRecordIndex;
        while (currentRecordIndex >= recordsInNode->size ())
        {
            currentRecordIndex = 0u;
            ++enumerator;
            recordsInNode = *enumerator;

            if (!recordsInNode)
            {
                // We're done: whole tree is scanned.
                return *static_cast<Inheritor *> (this);
            }
        }

        if (static_cast<Inheritor *> (this)->CheckIntersection ((*recordsInNode)[currentRecordIndex]))
        {
            // New intersection is found.
            return *static_cast<Inheritor *> (this);
        }
    }
}

template <typename Unit, std::size_t Dimensions>
template <typename Enumerator, typename Geometry, typename Inheritor>
Inheritor &VolumetricTree<Unit, Dimensions>::EnumeratorWrapper<Enumerator, Geometry, Inheritor>::operator~() noexcept
{
    const Container::Vector<const void *> *oldNode = *enumerator;
    EMERGENCE_ASSERT (oldNode);
    enumerator.EraseRecord (currentRecordIndex);
    const Container::Vector<const void *> *newNode = *enumerator;

    if (oldNode != newNode)
    {
        currentRecordIndex = 0u;
    }

    EnsureCurrentRecordIsValid ();
    return *static_cast<Inheritor *> (this);
}

template <typename Unit, std::size_t Dimensions>
template <typename Enumerator, typename Geometry, typename Inheritor>
VolumetricTree<Unit, Dimensions>::EnumeratorWrapper<Enumerator, Geometry, Inheritor>::EnumeratorWrapper (
    VolumetricTree *_tree, const Geometry &_geometry, Enumerator _enumerator) noexcept
    : tree (_tree),
      geometry (_geometry),
      enumerator (std::move (_enumerator))
{
    EnsureCurrentRecordIsValid ();
}

template <typename Unit, std::size_t Dimensions>
template <typename Enumerator, typename Geometry, typename Inheritor>
void VolumetricTree<Unit, Dimensions>::EnumeratorWrapper<Enumerator, Geometry, Inheritor>::
    EnsureCurrentRecordIsValid () noexcept
{
    if (*enumerator)
    {
        const void *initialRecord = **this;
        // If no records in root or no intersection with first record in root: move ahead.
        if (!initialRecord || !static_cast<Inheritor *> (this)->CheckIntersection (initialRecord))
        {
            ++*this;
        }
    }
}

template <typename Unit, std::size_t Dimensions>
VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator::ShapeIntersectionEnumerator (
    VolumetricTree *_tree,
    const VolumetricTree::Shape &_shape,
    typename PartitioningTree<Dimensions>::ShapeEnumerator _enumerator) noexcept
    : EnumeratorWrapper<typename PartitioningTree<Dimensions>::ShapeEnumerator, Shape, ShapeIntersectionEnumerator> (
          _tree, _shape, _enumerator)
{
}

template <typename Unit, std::size_t Dimensions>
bool VolumetricTree<Unit, Dimensions>::ShapeIntersectionEnumerator::CheckIntersection (
    const void *_record) const noexcept
{
    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        const Unit min = *static_cast<const Unit *> (this->tree->dimensions[index].minField.GetValue (_record));
        const Unit max = *static_cast<const Unit *> (this->tree->dimensions[index].maxField.GetValue (_record));

        if (max < this->geometry.bounds[index].min || min > this->geometry.bounds[index].max)
        {
            return false;
        }
    }

    return true;
}

template <typename Unit, std::size_t Dimensions>
VolumetricTree<Unit, Dimensions>::RayIntersectionEnumerator::RayIntersectionEnumerator (
    VolumetricTree *_tree,
    const LimitedFloatingRay &_ray,
    typename PartitioningTree<Dimensions>::RayEnumerator _enumerator) noexcept
    : EnumeratorWrapper<typename PartitioningTree<Dimensions>::RayEnumerator,
                        LimitedFloatingRay,
                        RayIntersectionEnumerator> (_tree, _ray, _enumerator)
{
}

template <typename Unit, std::size_t Dimensions>
bool VolumetricTree<Unit, Dimensions>::RayIntersectionEnumerator::CheckIntersection (const void *_record) const noexcept
{
    FloatingShape shape = this->tree->ConvertToFloatingShape (this->tree->ExtractShape (_record));

    // Algorithm is taken from GitHub:
    // https://github.com/erich666/GraphicsGems/blob/master/gems/RayBox.c

    constexpr std::uint8_t LEFT = 0u;
    constexpr std::uint8_t MIDDLE = 1u;
    constexpr std::uint8_t RIGHT = 2u;
    std::array<std::uint8_t, Dimensions> quadrant;

    bool insideShape = true;
    std::array<FloatingUnit, Dimensions> candidatePoint;

    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        if (this->geometry.axis[dimension].origin < shape.bounds[dimension].min)
        {
            quadrant[dimension] = LEFT;
            candidatePoint[dimension] = shape.bounds[dimension].min;
            insideShape = false;
        }
        else if (this->geometry.axis[dimension].origin > shape.bounds[dimension].max)
        {
            quadrant[dimension] = RIGHT;
            candidatePoint[dimension] = shape.bounds[dimension].max;
            insideShape = false;
        }
        else
        {
            quadrant[dimension] = MIDDLE;
        }
    }

    if (insideShape)
    {
        return true;
    }

    std::size_t selectedDimension = 0u;
    FloatingUnit maximumMovement = -1.0f;

    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        if (quadrant[dimension] != MIDDLE &&
            (this->geometry.axis[dimension].direction < -Constants::VolumetricIndex::EPSILON ||
             this->geometry.axis[dimension].direction > Constants::VolumetricIndex::EPSILON))
        {
            const FloatingUnit movement = (candidatePoint[dimension] - this->geometry.axis[dimension].origin) /
                                          this->geometry.axis[dimension].direction;

            if (maximumMovement < movement)
            {
                selectedDimension = dimension;
                maximumMovement = movement;
            }
        }
    }

    if (maximumMovement < static_cast<FloatingUnit> (0))
    {
        return false;
    }

    std::array<FloatingUnit, Dimensions> hitPoint;
    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        if (selectedDimension != dimension)
        {
            hitPoint[dimension] =
                this->geometry.axis[dimension].origin + this->geometry.axis[dimension].direction * maximumMovement;

            if (hitPoint[dimension] < shape.bounds[dimension].min || hitPoint[dimension] > shape.bounds[dimension].max)
            {
                return false;
            }
        }
        else
        {
            hitPoint[dimension] = candidatePoint[dimension];
        }
    }

    auto distanceSquared = static_cast<FloatingUnit> (0);
    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        const FloatingUnit difference = this->geometry.axis[dimension].origin - hitPoint[dimension];
        distanceSquared += difference * difference;
    }

    return distanceSquared <= this->geometry.lengthSquared;
}

template <typename Unit, std::size_t Dimensions>
VolumetricTree<Unit, Dimensions>::VolumetricTree (const std::array<Dimension, Dimensions> &_dimensions) noexcept
    : dimensions (_dimensions),
      partitioningTree (PreparePartitioningSpace (_dimensions))
{
}

template <typename Unit, std::size_t Dimensions>
const std::array<typename VolumetricTree<Unit, Dimensions>::Dimension, Dimensions> &
VolumetricTree<Unit, Dimensions>::GetDimensions () const noexcept
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
typename VolumetricTree<Unit, Dimensions>::RayIntersectionEnumerator
VolumetricTree<Unit, Dimensions>::EnumerateIntersectingShapes (const VolumetricTree::Ray &_ray,
                                                               VolumetricTree::FloatingUnit _maxLength) noexcept
{
    typename PartitioningTree<Dimensions>::Ray partitioningRay;
    std::array<float, Dimensions> partitioningDistanceFactors;

    LimitedFloatingRay ray;
    ray.lengthSquared = _maxLength * _maxLength;

    for (std::size_t dimension = 0u; dimension < Dimensions; ++dimension)
    {
        ray.axis[dimension].origin = static_cast<FloatingUnit> (_ray.axis[dimension].origin);
        ray.axis[dimension].direction = static_cast<FloatingUnit> (_ray.axis[dimension].direction);

        partitioningRay.axis[dimension].origin = ConvertPointToIndex (_ray.axis[dimension].origin, dimension);
        partitioningRay.axis[dimension].direction = ConvertDirectionToIndex (_ray.axis[dimension].direction, dimension);

        partitioningDistanceFactors[dimension] =
            static_cast<float> (dimensions[dimension].maxBorder - dimensions[dimension].minBorder) /
            static_cast<float> (partitioningTree.GetBorder ());
    }

    return {this, ray,
            partitioningTree.EnumerateIntersectingShapes (partitioningRay, static_cast<float> (_maxLength),
                                                          partitioningDistanceFactors)};
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
void VolumetricTree<Unit, Dimensions>::Clear () noexcept
{
    partitioningTree.Clear ();
}

template <typename Unit, std::size_t Dimensions>
typename PartitioningTree<Dimensions>::Index VolumetricTree<Unit, Dimensions>::PreparePartitioningSpace (
    const std::array<Dimension, Dimensions> &_dimensions) noexcept
{
    typename PartitioningTree<Dimensions>::Index maxBorder = 0u;
    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        const Unit space = _dimensions[index].maxBorder - _dimensions[index].minBorder;
        const float floatingPartitions =
            static_cast<float> (space) * Constants::VolumetricIndex::IDEAL_UNIT_TO_PARTITION_SCALE;

        const auto roundedPartitions =
            static_cast<typename PartitioningTree<Dimensions>::Index> (ceilf (floatingPartitions));

        constexpr std::size_t BIT_COUNT = sizeof (typename PartitioningTree<Dimensions>::Index) * 8u;
        maxBorder = std::max (maxBorder, static_cast<typename PartitioningTree<Dimensions>::Index> (
                                             1u << (BIT_COUNT - std::countl_zero (roundedPartitions))));
    }

    return maxBorder;
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
        convertedShape.bounds[index].min = ConvertPointToIndex (_shape.bounds[index].min, index);
        convertedShape.bounds[index].max = ConvertPointToIndex (_shape.bounds[index].max, index);
    }

    return convertedShape;
}

template <typename Unit, std::size_t Dimensions>
typename VolumetricTree<Unit, Dimensions>::FloatingShape VolumetricTree<Unit, Dimensions>::ConvertToFloatingShape (
    const VolumetricTree::Shape &_shape) const noexcept
{
    FloatingShape shape;
    for (std::size_t index = 0u; index < Dimensions; ++index)
    {
        shape.bounds[index].min = static_cast<FloatingUnit> (_shape.bounds[index].min);
        shape.bounds[index].max = static_cast<FloatingUnit> (_shape.bounds[index].max);
    }

    return shape;
}

template <typename Unit, std::size_t Dimensions>
typename PartitioningTree<Dimensions>::Index VolumetricTree<Unit, Dimensions>::ConvertPointToIndex (
    Unit _point, std::size_t _dimension) const noexcept
{
    const typename PartitioningTree<Dimensions>::Index partitions = partitioningTree.GetBorder ();
    const Unit space = dimensions[_dimension].maxBorder - dimensions[_dimension].minBorder;

    const Unit localizedValue = _point - dimensions[_dimension].minBorder;
    const FloatingUnit percent = static_cast<FloatingUnit> (localizedValue) / static_cast<FloatingUnit> (space);

    return static_cast<typename PartitioningTree<Dimensions>::Index> (
        floor (percent * static_cast<FloatingUnit> (partitions)));
}

template <typename Unit, std::size_t Dimensions>
typename PartitioningTree<Dimensions>::FloatingIndex VolumetricTree<Unit, Dimensions>::ConvertDirectionToIndex (
    Unit _direction, std::size_t _dimension) const noexcept
{
    const typename PartitioningTree<Dimensions>::Index partitions = partitioningTree.GetBorder ();
    const Unit space = dimensions[_dimension].maxBorder - dimensions[_dimension].minBorder;

    const FloatingUnit scale = static_cast<FloatingUnit> (partitions) / static_cast<FloatingUnit> (space);
    return static_cast<typename PartitioningTree<Dimensions>::FloatingIndex> (static_cast<FloatingUnit> (_direction) *
                                                                              scale);
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
    EMERGENCE_ASSERT (index == _other.index);
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
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

VolumetricIndex::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    VolumetricIndex::ShapeIntersectionReadCursor &&_other) noexcept
    : index (_other.index),
      baseEnumerator (std::move (_other.baseEnumerator))
{
    EMERGENCE_ASSERT (index);
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
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

VolumetricIndex::ShapeIntersectionEditCursor::ShapeIntersectionEditCursor (
    VolumetricIndex::ShapeIntersectionEditCursor &&_other) noexcept
    : index (_other.index),
      baseEnumerator (std::move (_other.baseEnumerator))
{
    EMERGENCE_ASSERT (index);
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
    EMERGENCE_ASSERT (index);
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
    : index (_other.index),
      baseEnumerator (_other.baseEnumerator)
{
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

VolumetricIndex::RayIntersectionReadCursor::RayIntersectionReadCursor (
    VolumetricIndex::RayIntersectionReadCursor &&_other) noexcept
    : index (_other.index),
      baseEnumerator (std::move (_other.baseEnumerator))
{
    EMERGENCE_ASSERT (index);
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
    return std::visit (
        [] (const auto &_enumerator)
        {
            return *_enumerator;
        },
        baseEnumerator);
}

VolumetricIndex::RayIntersectionReadCursor &VolumetricIndex::RayIntersectionReadCursor::operator++ () noexcept
{
    std::visit (
        [] (auto &_enumerator)
        {
            ++_enumerator;
        },
        baseEnumerator);
    return *this;
}

VolumetricIndex::RayIntersectionReadCursor::RayIntersectionReadCursor (
    VolumetricIndex *_index, RayIntersectionEnumeratorVariant _baseEnumerator) noexcept
    : index (_index),
      baseEnumerator (std::move (_baseEnumerator))
{
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

VolumetricIndex::RayIntersectionEditCursor::RayIntersectionEditCursor (
    VolumetricIndex::RayIntersectionEditCursor &&_other) noexcept
    : index (_other.index),
      baseEnumerator (std::move (_other.baseEnumerator))
{
    EMERGENCE_ASSERT (index);
    _other.index = nullptr;
}

VolumetricIndex::RayIntersectionEditCursor::~RayIntersectionEditCursor () noexcept
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

void *VolumetricIndex::RayIntersectionEditCursor::operator* () noexcept
{
    const void *record = std::visit (
        [] (const auto &_enumerator)
        {
            return *_enumerator;
        },
        baseEnumerator);
    return const_cast<void *> (record);
}

VolumetricIndex::RayIntersectionEditCursor &VolumetricIndex::RayIntersectionEditCursor::operator++ () noexcept
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

VolumetricIndex::RayIntersectionEditCursor &VolumetricIndex::RayIntersectionEditCursor::operator~() noexcept
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

VolumetricIndex::RayIntersectionEditCursor::RayIntersectionEditCursor (
    VolumetricIndex *_index, RayIntersectionEnumeratorVariant _baseEnumerator) noexcept
    : index (_index),
      baseEnumerator (std::move (_baseEnumerator))
{
    EMERGENCE_ASSERT (index);
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
void VolumetricIndex::RayIntersectionEditCursor::BeginRecordEdition (Enumerator &_enumerator) noexcept
{
    if (const void *record = *_enumerator)
    {
        index->storage->BeginRecordEdition (record);
    }
}

template <typename Enumerator>
bool VolumetricIndex::RayIntersectionEditCursor::EndRecordEdition (Enumerator &_enumerator) noexcept
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

VolumetricIndex::RayIntersectionReadCursor VolumetricIndex::LookupRayIntersectionToRead (const void *_ray,
                                                                                         float _rayLength) noexcept
{
    RayIntersectionEnumeratorVariant enumerator = std::visit (
        [_ray, _rayLength] (auto &_tree) -> RayIntersectionEnumeratorVariant
        {
            using Tree = std::decay_t<decltype (_tree)>;
            return _tree.EnumerateIntersectingShapes (*static_cast<const typename Tree::Ray *> (_ray), _rayLength);
        },
        tree);
    return {this, std::move (enumerator)};
}

VolumetricIndex::RayIntersectionEditCursor VolumetricIndex::LookupRayIntersectionToEdit (const void *_ray,
                                                                                         float _rayLength) noexcept
{
    RayIntersectionEnumeratorVariant enumerator = std::visit (
        [_ray, _rayLength] (auto &_tree) -> RayIntersectionEnumeratorVariant
        {
            using Tree = std::decay_t<decltype (_tree)>;
            return _tree.EnumerateIntersectingShapes (*static_cast<const typename Tree::Ray *> (_ray), _rayLength);
        },
        tree);
    return {this, std::move (enumerator)};
}

void VolumetricIndex::Drop () noexcept
{
    EMERGENCE_ASSERT (CanBeDropped ());
    EMERGENCE_ASSERT (storage);
    storage->DropIndex (*this);
}

template <typename Unit, std::size_t Count>
std::array<typename VolumetricTree<Unit, Count>::Dimension, Count> ConvertDimensions (
    Storage *_storage, const Container::Vector<VolumetricIndex::DimensionDescriptor> &_dimensions)
{
    EMERGENCE_ASSERT (_dimensions.size () == Count);
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
    EMERGENCE_ASSERT (!_dimensions.empty ());
    EMERGENCE_ASSERT (_dimensions.size () <= Constants::VolumetricIndex::MAX_DIMENSIONS);

    StandardLayout::Field selectionBaseField = _storage->GetRecordMapping ().GetField (_dimensions.front ().minField);
    EMERGENCE_ASSERT (selectionBaseField.GetArchetype () == StandardLayout::FieldArchetype::INT ||
                      selectionBaseField.GetArchetype () == StandardLayout::FieldArchetype::UINT ||
                      selectionBaseField.GetArchetype () == StandardLayout::FieldArchetype::FLOAT);

#define UNHANDLED_COMBINATION                                                                                          \
    EMERGENCE_ASSERT (false);                                                                                          \
    return VolumetricTree<std::uint8_t, 1u> ({})

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
            break;                                                                                                     \
                                                                                                                       \
        case StandardLayout::FieldArchetype::UINT:                                                                     \
            switch (selectionBaseField.GetSize ())                                                                     \
            {                                                                                                          \
            case 1u:                                                                                                   \
                return VolumetricTree<std::uint8_t, Count> (                                                           \
                    ConvertDimensions<std::uint8_t, Count> (_storage, _dimensions));                                   \
            case 2u:                                                                                                   \
                return VolumetricTree<std::uint16_t, Count> (                                                          \
                    ConvertDimensions<std::uint16_t, Count> (_storage, _dimensions));                                  \
            case 4u:                                                                                                   \
                return VolumetricTree<std::uint32_t, Count> (                                                          \
                    ConvertDimensions<std::uint32_t, Count> (_storage, _dimensions));                                  \
            case 8u:                                                                                                   \
                return VolumetricTree<std::uint64_t, Count> (                                                          \
                    ConvertDimensions<std::uint64_t, Count> (_storage, _dimensions));                                  \
            }                                                                                                          \
                                                                                                                       \
            break;                                                                                                     \
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
            break;                                                                                                     \
                                                                                                                       \
        case StandardLayout::FieldArchetype::BIT:                                                                      \
        case StandardLayout::FieldArchetype::STRING:                                                                   \
        case StandardLayout::FieldArchetype::BLOCK:                                                                    \
        case StandardLayout::FieldArchetype::UNIQUE_STRING:                                                            \
        case StandardLayout::FieldArchetype::UTF8_STRING:                                                              \
        case StandardLayout::FieldArchetype::NESTED_OBJECT:                                                            \
        case StandardLayout::FieldArchetype::VECTOR:                                                                   \
        case StandardLayout::FieldArchetype::PATCH:                                                                    \
            UNHANDLED_COMBINATION;                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        break;

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

void VolumetricIndex::Clear () noexcept
{
    EMERGENCE_ASSERT (reinsertionQueue.empty ());
    std::visit (
        [] (auto &_tree)
        {
            _tree.Clear ();
        },
        tree);
}
} // namespace Emergence::Pegasus
