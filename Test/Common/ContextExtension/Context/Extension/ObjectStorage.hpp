#pragma once

#include <string>
#include <unordered_map>

#include <Testing/Testing.hpp>

namespace Emergence::Context::Extension
{
/// \brief Scenarios for high-level libraries could use more than one object type, therefore templated
///        tasks were added in order to make integration with multiple object types easier.
namespace Tasks
{
template <typename Object>
struct Move
{
    std::string sourceName;
    std::string targetName;
};

template <typename Object>
struct Copy
{
    std::string sourceName;
    std::string targetName;
};

template <typename Object>
struct MoveAssign
{
    std::string sourceName;
    std::string targetName;
};

template <typename Object>
struct CopyAssign
{
    std::string sourceName;
    std::string targetName;
};

template <typename Object>
struct Delete
{
    std::string name;
};
} // namespace Tasks

/// \brief Stores objects of given type and supports Tasks execution.
/// \details Designed to be drop-in extension for test execution contexts of high level library scenarios.
template <typename Object>
struct ObjectStorage
{
    std::unordered_map <std::string, Object> objects;
};

template <typename Object, typename... Types>
void AddObject (ObjectStorage <Object> &_storage, std::string _name, Types &&... _constructionArguments)
{
    REQUIRE_WITH_MESSAGE (_storage.objects.find (_name) == _storage.objects.end (),
                          "Object \"", _name, "\" should not exist!");
    _storage.objects.emplace (std::move (_name), Object (std::forward <Types> (_constructionArguments)...));
}

template <typename Object>
Object &GetObject (ObjectStorage <Object> &_storage, const std::string &_name)
{
    auto iterator = _storage.objects.find (_name);
    REQUIRE_WITH_MESSAGE (iterator != _storage.objects.end (), "Object \"", _name, "\" should exist!");
    return iterator->second;
}

template <typename Object>
void ExecuteTask (ObjectStorage <Object> &_storage, const Tasks::Move <Object> &_task)
{
    Object &source = GetObject (_storage, _task.sourceName);
    if constexpr (std::is_move_constructible_v <Object>)
    {
        AddObject (_storage, _task.targetName, std::move (source));
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Object type must be movable!");
    }
}

template <typename Object>
void ExecuteTask (ObjectStorage <Object> &_storage, const Tasks::Copy <Object> &_task)
{
    Object &source = GetObject (_storage, _task.sourceName);
    if constexpr (std::is_copy_constructible_v <Object>)
    {
        AddObject (_storage, _task.targetName, source);
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Object type must be copyable!");
    }
}

template <typename Object>
void ExecuteTask (ObjectStorage <Object> &_storage, const Tasks::MoveAssign <Object> &_task)
{
    Object &source = GetObject (_storage, _task.sourceName);
    Object &target = GetObject (_storage, _task.targetName);

    if constexpr (std::is_move_assignable_v <Object>)
    {
        target = std::move (source);
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Object type must be movable!");
    }
}

template <typename Object>
void ExecuteTask (ObjectStorage <Object> &_storage, const Tasks::CopyAssign <Object> &_task)
{
    Object &source = GetObject (_storage, _task.sourceName);
    Object &target = GetObject (_storage, _task.targetName);

    if constexpr (std::is_copy_assignable_v <Object>)
    {
        target = source;
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Object type must be movable!");
    }
}

template <typename Object>
void ExecuteTask (ObjectStorage <Object> &_storage, const Tasks::Delete <Object> &_task)
{
    auto iterator = _storage.objects.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != _storage.objects.end (), "Object \"", _task.name, "\" should exist!");
    _storage.objects.erase (iterator);
}
} // namespace Emergence::Context::Extension

#define EMERGENCE_TEST_OBJECT_STORAGE_TASK_EXECUTION_DEDUCTION_HELPER(HighLevelContext, ObjectType)                    \
void ExecuteTask (                                                                                                     \
    HighLevelContext &_context, const Emergence::Context::Extension::Tasks::Move <ObjectType> &_task)                  \
{                                                                                                                      \
    Emergence::Context::Extension::ExecuteTask <ObjectType> (_context, _task);                                         \
}                                                                                                                      \
                                                                                                                       \
void ExecuteTask (                                                                                                     \
    HighLevelContext &_context, const Emergence::Context::Extension::Tasks::Copy <ObjectType> &_task)                  \
{                                                                                                                      \
    Emergence::Context::Extension::ExecuteTask <ObjectType> (_context, _task);                                         \
}                                                                                                                      \
                                                                                                                       \
void ExecuteTask (                                                                                                     \
    HighLevelContext &_context, const Emergence::Context::Extension::Tasks::MoveAssign <ObjectType> &_task)            \
{                                                                                                                      \
    Emergence::Context::Extension::ExecuteTask <ObjectType> (_context, _task);                                         \
}                                                                                                                      \
                                                                                                                       \
void ExecuteTask (                                                                                                     \
    HighLevelContext &_context, const Emergence::Context::Extension::Tasks::CopyAssign <ObjectType> &_task)            \
{                                                                                                                      \
    Emergence::Context::Extension::ExecuteTask <ObjectType> (_context, _task);                                         \
}                                                                                                                      \
                                                                                                                       \
void ExecuteTask (                                                                                                     \
    HighLevelContext &_context, const Emergence::Context::Extension::Tasks::Delete <ObjectType> &_task)                \
{                                                                                                                      \
    Emergence::Context::Extension::ExecuteTask <ObjectType> (_context, _task);                                         \
}