#pragma once

namespace Emergence
{
/// \brief Helper structure for ::SelectType.
template <typename IfTrue, typename IfFalse, bool selector>
struct SelectTypeStruct;

/// \brief Helper structure for ::SelectType. Implements true selection.
template <typename IfTrue, typename IfFalse>
struct SelectTypeStruct<IfTrue, IfFalse, true>
{
    using Result = IfTrue;
};

/// \brief Helper structure for ::SelectType. Implements false selection.
template <typename IfTrue, typename IfFalse>
struct SelectTypeStruct<IfTrue, IfFalse, false>
{
    using Result = IfFalse;
};

/// \brief Returns first type if given boolean variable is true. Otherwise returns second type.
template <typename IfTrue, typename IfFalse, bool selector>
using SelectType = typename SelectTypeStruct<IfTrue, IfFalse, selector>::Result;
} // namespace Emergence
