#include <cassert>
#include <optional>

#include <Galleon/Test/Common.hpp>

namespace Emergence::Galleon::Test
{
void ExecuteReferenceApiTest (const Reference::Test::Scenario &_scenario,
                              std::function <std::vector <Task> (const std::string &, bool)> _onCreate,
                              std::function <std::vector <Task> (bool)> _onStatusCheck,
                              bool _containerReferenceBaseImporter)
{
    std::vector <Emergence::Galleon::Test::Task> tasks;
    bool isAlreadyInitializedTrivialExpectation = false;

    for (const Emergence::Reference::Test::Task &_packedTask : _scenario)
    {
        std::visit (
            [&tasks, &isAlreadyInitializedTrivialExpectation,
                _containerReferenceBaseImporter, &_onCreate, &_onStatusCheck] (
                const auto &_task)
            {
                using TaskType = std::decay_t <decltype (_task)>;
                if constexpr (std::is_same_v <TaskType, Emergence::Reference::Test::Tasks::Create>)
                {
                    tasks += _onCreate (_task.name, isAlreadyInitializedTrivialExpectation);
                    isAlreadyInitializedTrivialExpectation = true;
                }
                else if constexpr (std::is_same_v <TaskType, Emergence::Reference::Test::Tasks::CheckStatus>)
                {
                    isAlreadyInitializedTrivialExpectation = _task.hasAnyReferences;
                    tasks += _onStatusCheck (_task.hasAnyReferences);
                }
                else
                {
                    if (_containerReferenceBaseImporter)
                    {
                        tasks.emplace_back (TestReferenceApiImporters::ForContainerReference (_task));
                    }
                    else
                    {
                        tasks.emplace_back (TestReferenceApiImporters::ForPreparedQuery (_task));
                    }
                }
            },
            _packedTask);
    }

    Scenario {tasks};
}

static Task RenamePreparedQuery (Task _queryPreparation, std::string _newContainerName, std::string _newQueryName)
{
    std::visit (
        [&_newContainerName, &_newQueryName] (auto &_task)
        {
            if constexpr (std::is_base_of_v <QueryPreparationBase, std::decay_t <decltype (_task)>>)
            {
                _task.containerName = _newContainerName;
                _task.queryName = _newQueryName;
            }
            else
            {
                REQUIRE_WITH_MESSAGE (false, "Query preparation task should inherit QueryPreparationBase.");
            }
        },
        _queryPreparation);

    return _queryPreparation;
}

void AdaptPreparedQueryReferenceApiTest (
    const Reference::Test::Scenario &_scenario, const Task &_prepareQuery,
    const std::function <std::vector <Task> (const std::string &, bool)> &_onContainerCreate,
    const std::function <std::vector <Task> (bool)> &_onStatusCheck)
{
    ExecuteReferenceApiTest (
        _scenario,
        [&_prepareQuery, &_onContainerCreate] (
            const std::string &_name, bool _isAlreadyInitializedTrivialExpectation) -> std::vector <Task>
        {
            return _onContainerCreate (
                _name + "TemporaryContainer", _isAlreadyInitializedTrivialExpectation) +
                   RenamePreparedQuery (_prepareQuery, _name + "TemporaryContainer", _name) +
                   Delete < ContainerReferenceTag > {_name + "TemporaryContainer"};
        },
        _onStatusCheck,
        false);
}
} // namespace Emergence::Galleon::Test