#pragma once

#include <CommandLineApi.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::CommandLine
{
/// \brief Parses command line arguments into given output object according to reflection.
/// \details Expects arguments to be in format: executable --fieldName fieldValue ... --fieldName fieldValue.
///          Left without tests, because covering it fully would require much more code than implementation
///          and implementation is rather straightforward, so we can be quite sure that it is fine.
bool CommandLineApi ParseArguments (int _argumentCount,
                                    char **_arguments,
                                    void *_output,
                                    const StandardLayout::Mapping &_mapping) noexcept;
} // namespace Emergence::CommandLine
