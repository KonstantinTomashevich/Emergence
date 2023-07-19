#pragma once

#include <CelerityUIApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Building block of UI hierarchy and socket for control insertion.
struct CelerityUIApi UINode final
{
    /// \brief Unique id of this node.
    /// \invariant Generated through UISingleton::GenerateNodeId.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Id of a parent node in hierarchy, if any.
    UniqueId parentId = INVALID_UNIQUE_ID;

    /// \brief Style that should be applied to this node and its children.
    /// \details If has default value, no style will be applied.
    ///          Children may select their styles and this will result in style blending: children
    ///          style values will overwrite parent style values for this children sub tree.
    Memory::UniqueString styleId;

    /// \brief Sort indices are used to determinate order of nodes that have the same parent.
    /// \details For example, we need to position 3 buttons from left to right in horizontal container.
    ///          In this case leftmost node should have the smallest sort index and rightmost should
    ///          have the biggest one.
    std::uint64_t sortIndex = 0u;

    struct CelerityUIApi Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId parentId;
        StandardLayout::FieldId styleId;
        StandardLayout::FieldId sortIndex;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
