#pragma once

#include <functional>

#include <Galleon/Test/Scenario.hpp>

namespace Emergence::Galleon::Test
{
void ExecuteReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario,
    std::function <std::vector <Task> (const std::string &, bool)> _onCreate,
    std::function <std::vector <Task> (bool)> _onStatusCheck,
    bool _containerReferenceBaseImporter);

void AdaptPreparedQueryReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario,
    const Task& _prepareQuery,
    const std::function <std::vector <Task> (const std::string &, bool)>& _onContainerCreate,
    const std::function <std::vector <Task> (bool)>& _onStatusCheck);
} // namespace Emergence::Galleon::Test