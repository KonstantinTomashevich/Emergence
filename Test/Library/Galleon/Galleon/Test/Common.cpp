#include <cassert>
#include <optional>

#include <Galleon/Test/Common.hpp>

namespace Emergence::Galleon::Test
{
static std::string ExtractContainerName (const std::vector <Task> &_containerInitializer)
{
    for (const Task &packagedTask : _containerInitializer)
    {
        const std::optional <std::string> containerName = std::visit (
            [] (const auto &_task) -> std::optional <std::string>
            {
                using TaskType = std::decay_t <decltype (_task)>;
                if constexpr (std::is_base_of_v <ContainerAcquisitionBase, TaskType>)
                {
                    return _task.name;
                }
                else
                {
                    return std::nullopt;
                }
            },
            packagedTask);

        if (containerName.has_value ())
        {
            return containerName.value ();
        }
    }

    assert (false);
    return "Unable to extract container name!";
}

static std::string ExtractQueryName (const Task &_queryPreparation)
{
    return std::visit (
        [] (const auto &_task) -> std::string
        {
            using TaskType = std::decay_t <decltype (_task)>;
            if constexpr (std::is_base_of_v <QueryPreparationBase, TaskType>)
            {
                return _task.queryName;
            }
            else
            {
                assert (false);
                return "Unable to extract query name!";
            }
        },
        _queryPreparation);
}

std::vector <Task> TestContainerReferenceManipulation (
    const std::vector <Task> &_initContainer, const std::vector <Task> &_checkThatContainerInitialized,
    const std::vector <Task> &_checkThatContainerNotInitialized)
{
    std::string containerName = ExtractContainerName (_initContainer);
    assert (containerName != "moved");
    assert (containerName != "copied");

    return _checkThatContainerNotInitialized +
           _initContainer +
           _checkThatContainerInitialized +
           MoveContainerReference {containerName, "moved"} +
           _checkThatContainerInitialized +
           CopyContainerReference {"moved", "copied"} +
           RemoveContainerReference {"moved"} +
           _checkThatContainerInitialized +
           RemoveContainerReference {"copied"} +
           _checkThatContainerNotInitialized;
}

std::vector <Task> TestQueryReferenceManipulation (
    const std::vector <Task> &_initContainer,
    const Task &_prepareQuery,
    const std::vector <Task> &_checkThatContainerInitialized,
    const std::vector <Task> &_checkThatContainerNotInitialized)
{
    std::string containerName = ExtractContainerName (_initContainer);
    std::string queryName = ExtractQueryName (_prepareQuery);

    assert (queryName != "moved");
    assert (queryName != "copied");

    return _checkThatContainerNotInitialized +
           _initContainer +
           _checkThatContainerInitialized +
           _prepareQuery +
           RemoveContainerReference {containerName} +
           _checkThatContainerInitialized +
           MovePreparedQuery {queryName, "moved"} +
           _checkThatContainerInitialized +
           CopyPreparedQuery {"moved", "copied"} +
           RemovePreparedQuery {"moved"} +
           _checkThatContainerInitialized +
           RemovePreparedQuery {"copied"} +
           _checkThatContainerNotInitialized;
}
} // namespace Emergence::Galleon::Test