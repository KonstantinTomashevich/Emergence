#pragma once

#include <istream>
#include <ostream>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Serialization::Binary
{
void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept;

bool DeserializeObject (std::istream &_input, void *_object, const StandardLayout::Mapping &_mapping) noexcept;
} // namespace Emergence::Serialization::Binary
