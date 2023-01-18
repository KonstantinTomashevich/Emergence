#pragma once

#include <Container/String.hpp>

namespace Emergence::FileSystem::Test
{
/// \brief Compares contents of given binary files and fails test if they are not equal.
/// \details No-exception checks are used for mismatches, so execution will be continued if files are different.
///          But if any file does not exists or an IO error occurs, test will be aborted.
void ExpectFilesEqual (const Container::String &_expectedFilePath, const Container::String &_resultFilePath) noexcept;
} // namespace Emergence::FileSystem::Test
