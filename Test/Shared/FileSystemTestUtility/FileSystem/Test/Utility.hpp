#pragma once

#include <Container/String.hpp>

namespace Emergence::FileSystem::Test
{
/// \brief Compares contents of given binary files and fails test if file lengths
///        are not equal or amount of different bytes is higher than given ratio.
void CheckFilesEquality (const Container::String &_expectedFilePath,
                         const Container::String &_resultFilePath,
                         float _maximumErrorRatio) noexcept;
} // namespace Emergence::FileSystem::Test
