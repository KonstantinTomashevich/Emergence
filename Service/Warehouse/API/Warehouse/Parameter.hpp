#pragma once

namespace Emergence::Warehouse
{
/// \brief Defines sequence of values by specifying value for each key field.
///
/// \details Key field count and types are unknown during compile time, therefore ValueSequence is
///          a pointer to memory block, that holds values for each key field in correct order.
///
/// \warning Due to runtime-only nature of sequences, logically incorrect pointers can not be caught.
/// \invariant Should not be `nullptr`.
/// \invariant Values must be stored one after another without paddings in the same order as key fields.
/// \invariant Value size for fields with StandardLayout::FieldArchetype::STRING must always be equal to
///            StandardLayout::Field::GetSize, even if string length is less than this value.
/// \invariant Values for fields with StandardLayout::FieldArchetype::BIT must passed as bytes in which all
///            bits should be zero's except bit with StandardLayout::Field::GetBitOffset.
using ValueSequence = const void *;

/// \brief Points to value, that defines one of interval bounds for range lookup.
///
/// \details Key field type is unknown during compile time, therefore value is a pointer to memory
///          block with actual value. `nullptr` values will be interpreted as absence of borders.
///
/// \warning Due to runtime-only nature of values, logically incorrect pointers can not be caught.
using Bound = const void *;

/// \brief Defines shape by specifying min-max value pair for each dimension.
///
/// \details Dimension count and types are unknown during compile time, therefore Shape is a pointer to
///          memory block, that holds min-max pair of values for each dimension in correct order. For example,
///          if it's needed to describe rectangle with width equal to 3, height equal to 2, center in
///          (x = 1, y = 3} point and dimensions are x = {float x0; float x1;} and y = {float y0; float y1;},
///          then shape memory block should be {-0.5f, 2.5f, 2.0f, 4.0f}.
///
/// \warning Due to runtime-only nature of shapes, logically incorrect pointers can not be caught.
/// \invariant Should not be `nullptr`.
using Shape = const void *;

/// \brief Defines ray by specifying origin-direction value pair for each dimension.
///
/// \details Dimension count and types are unknown during compile time, therefore Ray is a pointer to
///          memory block, that holds origin-direction pair of values for each dimension in correct order.
///          For example, if it's needed to describe ray with origin in (x = 2, y = 3) point,
///          (dx = 0.8, dy = -0.6) direction and dimensions are x = {float x0; float x1;} and
///          y = {float y0; float y1;}, then ray memory block should be {2.0f, 0.8f, 3.0f, -0.6f}.
///
/// \warning Due to runtime-only nature of rays, logically incorrect pointers can not be caught.
/// \invariant Should not be `nullptr`.
using Ray = const void *;
} // namespace Emergence::Warehouse