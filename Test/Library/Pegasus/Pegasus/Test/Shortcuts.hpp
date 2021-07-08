#pragma once

#include <Pegasus/Test/Scenario.hpp>

namespace Emergence::Pegasus::Test::Shortcuts
{
std::vector <Task> TestIsCanBeDropped (const std::string &_indexName);

std::vector <Task> TestCursorCopyAndMove (
    const Task &_readCursorConstructor, const Task &_editCursorConstructor,
    const void *_readCursorExpectedFirstRecord, const void *_readCursorExpectedSecondRecord,
    const void *_editCursorExpectedFirstRecord);
} // namespace Emergence::Pegasus::Test::Common
