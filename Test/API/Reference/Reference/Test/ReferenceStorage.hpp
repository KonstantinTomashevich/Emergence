#pragma once

#include <string>
#include <unordered_map>

#include <Reference/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Reference::Test
{
/// \brief Scenarios for high-level libraries could contain more than one reference type, therefore templated
///        tasks were added in order to make integration with multiple reference types easier.
namespace TemplatedTasks
{
template <typename Reference>
struct Move : public Tasks::Move
{
};

template <typename Reference>
struct Copy : public Tasks::Copy
{
};

template <typename Reference>
struct MoveAssign : public Tasks::MoveAssign
{
};

template <typename Reference>
struct CopyAssign : public Tasks::CopyAssign
{
};

template <typename Reference>
struct Delete : public Tasks::Delete
{
};
} // namespace TemplatedTasks

/// \brief Stores references of given types and supports TemplatedTasks execution.
/// \details Designed to be drop-in extension for test execution contexts of high level library scenarios.
template <typename Reference>
struct ReferenceStorage
{
    std::unordered_map <std::string, Reference> references;
};

template <typename Reference, typename... Types>
void AddReference (ReferenceStorage <Reference> &_storage, std::string _name, Types &&... _constructionArguments)
{
    REQUIRE_WITH_MESSAGE (_storage.references.find (_name) == _storage.references.end (),
                          "Reference \"", _name, "\" should not exist!");
    _storage.references.emplace (std::move (_name), Reference (std::forward <Types> (_constructionArguments)...));
}

template <typename Reference>
Reference &GetReference (ReferenceStorage <Reference> &_storage, const std::string &_name)
{
    auto iterator = _storage.references.find (_name);
    REQUIRE_WITH_MESSAGE (iterator != _storage.references.end (), "Reference \"", _name, "\" should exist!");
    return iterator->second;
}

template <typename Reference>
void ExecuteTask (ReferenceStorage <Reference> &_storage, const TemplatedTasks::Move <Reference> &_task)
{
    Reference &source = GetReference (_storage, _task.source);
    if constexpr (std::is_move_constructible_v <Reference>)
    {
        AddReference (_storage, _task.target, std::move (source));
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Reference type must be movable!");
    }
}

template <typename Reference>
void ExecuteTask (ReferenceStorage <Reference> &_storage, const TemplatedTasks::Copy <Reference> &_task)
{
    Reference &source = GetReference (_storage, _task.source);
    if constexpr (std::is_copy_constructible_v <Reference>)
    {
        AddReference (_storage, _task.target, source);
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Reference type must be copyable!");
    }
}

template <typename Reference>
void ExecuteTask (ReferenceStorage <Reference> &_storage, const TemplatedTasks::MoveAssign <Reference> &_task)
{
    Reference &source = GetReference (_storage, _task.source);
    Reference &target = GetReference (_storage, _task.target);

    if constexpr (std::is_move_constructible_v <Reference>)
    {
        target = std::move (source);
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Reference type must be movable!");
    }
}

template <typename Reference>
void ExecuteTask (ReferenceStorage <Reference> &_storage, const TemplatedTasks::CopyAssign <Reference> &_task)
{
    Reference &source = GetReference (_storage, _task.source);
    Reference &target = GetReference (_storage, _task.target);

    if constexpr (std::is_copy_constructible_v <Reference>)
    {
        target = source;
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Reference type must be movable!");
    }
}

template <typename Reference>
void ExecuteTask (ReferenceStorage <Reference> &_storage, const TemplatedTasks::Delete <Reference> &_task)
{
    auto iterator = _storage.references.find (_task.name);
    REQUIRE_WITH_MESSAGE (iterator != _storage.references.end (), "Reference \"", _task.name, "\" should exist!");
    _storage.references.erase (iterator);
}
} // namespace Emergence::Reference::Test