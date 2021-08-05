#pragma once

#include <Galleon/Test/Scenario.hpp>

namespace Emergence::Galleon::Test
{
std::vector <Task> TestContainerReferenceManipulation (
    const std::vector <Task> &_initContainer,
    const std::vector <Task> &_checkThatContainerInitialized,
    const std::vector <Task> &_checkThatContainerNotInitialized);

std::vector <Task> TestQueryReferenceManipulation (
    const std::vector <Task> &_initContainer,
    const Task &_prepareQuery,
    const std::vector <Task> &_checkThatContainerInitialized,
    const std::vector <Task> &_checkThatContainerNotInitialized);
} // namespace Emergence::Galleon::Test