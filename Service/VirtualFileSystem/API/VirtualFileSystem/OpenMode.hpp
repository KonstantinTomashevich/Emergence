#pragma once

namespace Emergence::VirtualFileSystem
{
/// \brief Describes in which mode file should be opened.
enum class OpenMode
{
    // TODO: It seems impossible to adequately support text reading mode. Cut it out?
    BINARY = 0u,
    TEXT,
};
} // namespace Emergence::VirtualFileSystem
