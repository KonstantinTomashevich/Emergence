#pragma once

#include <Query/Test/Scenario.hpp>

namespace Emergence::Query::Test
{
std::vector <Task> TestCursorCopyAndMove (
    const Task &_readCursorQuery, const Task &_editCursorQuery,
    const void *_readCursorExpectedFirstObject, const void *_readCursorExpectedSecondObject,
    const void *_editCursorExpectedFirstObject);
} // namespace Emergence::Query::Test