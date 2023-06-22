#pragma once

#include <iostream>

#include <Container/String.hpp>

namespace Emergence::Testing
{
/// \brief Compares contents of given binary streams and fails test if their lengths
///        are not equal or amount of different bytes is higher than given ratio.
void CheckStreamEquality (std::istream &_expectedInput, std::istream &_resultInput, float _maximumErrorRatio) noexcept;
} // namespace Emergence::Testing
