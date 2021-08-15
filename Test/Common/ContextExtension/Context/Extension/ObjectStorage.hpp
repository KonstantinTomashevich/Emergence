#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include <Testing/Testing.hpp>

namespace Emergence::Context::Extension
{
/// \brief Scenarios for high-level libraries could use more than one object type, therefore templated
///        tasks were added in order to make integration with multiple object types easier.
/// \details Instead of real object types, user should pass tag-types (for example empty structs) as template arguments
///          for these tasks. This approach both helps to reduce compilation time by not including real types to task
///          declaration headers and helps compiler to deduce task executors. Tag-to-object-type binding should be
///          registered via EMERGENCE_CONTEXT_BIND_OBJECT_TAG.
namespace Tasks
{
template <typename ObjectTag>
struct Move
{
    std::string sourceName;
    std::string targetName;
};

template <typename ObjectTag>
struct Copy
{
    std::string sourceName;
    std::string targetName;
};

template <typename ObjectTag>
struct MoveAssign
{
    std::string sourceName;
    std::string targetName;
};

template <typename ObjectTag>
struct CopyAssign
{
    std::string sourceName;
    std::string targetName;
};

template <typename ObjectTag>
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
void RemoveObject (ObjectStorage <Object> &_storage, const std::string &_name)
{
    auto iterator = _storage.objects.find (_name);
    REQUIRE_WITH_MESSAGE (iterator != _storage.objects.end (), "Object \"", _name, "\" should exist!");
    _storage.objects.erase (iterator);
}

template <typename ObjectTag>
struct ObjectTagMeta
{
    // This type must always be specialized, therefore absence of `using Object = ...` is intentional.
};

template <typename ObjectTag>
using ObjectFromTag = typename ObjectTagMeta <ObjectTag>::Object;

template <typename ObjectTag>
using ObjectStorageFromTag = ObjectStorage <ObjectFromTag <ObjectTag>>;

template <typename ObjectTag>
void ExecuteTask (ObjectStorageFromTag <ObjectTag> &_storage, const Tasks::Move <ObjectTag> &_task)
{
    ObjectFromTag <ObjectTag> &source = GetObject (_storage, _task.sourceName);
    if constexpr (std::is_move_constructible_v <ObjectFromTag <ObjectTag>>)
    {
        AddObject (_storage, _task.targetName, std::move (source));
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Object type must be movable!");
    }
}

template <typename ObjectTag>
void ExecuteTask (ObjectStorageFromTag <ObjectTag> &_storage, const Tasks::Copy <ObjectTag> &_task)
{
    ObjectFromTag <ObjectTag> &source = GetObject (_storage, _task.sourceName);
    if constexpr (std::is_copy_constructible_v <ObjectFromTag <ObjectTag>>)
    {
        AddObject (_storage, _task.targetName, source);
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Object type must be copyable!");
    }
}

template <typename ObjectTag>
void ExecuteTask (ObjectStorageFromTag <ObjectTag> &_storage, const Tasks::MoveAssign <ObjectTag> &_task)
{
    ObjectFromTag <ObjectTag> &source = GetObject (_storage, _task.sourceName);
    ObjectFromTag <ObjectTag> &target = GetObject (_storage, _task.targetName);

    if constexpr (std::is_move_assignable_v <ObjectFromTag <ObjectTag>>)
    {
        target = std::move (source);
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Object type must be movable!");
    }
}

template <typename ObjectTag>
void ExecuteTask (ObjectStorageFromTag <ObjectTag> &_storage, const Tasks::CopyAssign <ObjectTag> &_task)
{
    ObjectFromTag <ObjectTag> &source = GetObject (_storage, _task.sourceName);
    ObjectFromTag <ObjectTag> &target = GetObject (_storage, _task.targetName);

    if constexpr (std::is_copy_assignable_v <ObjectFromTag <ObjectTag>>)
    {
        target = source;
    }
    else
    {
        REQUIRE_WITH_MESSAGE (false, "Object type must be movable!");
    }
}

namespace Tasks
{
template <typename ObjectTag>
void ExecuteTask (ObjectStorageFromTag <ObjectTag> &_storage, const Tasks::Delete <ObjectTag> &_task)
{
    RemoveObject (_storage, _task.name);
}

template <typename ObjectTag>
std::ostream &operator << (std::ostream &_output, const Tasks::Move <ObjectTag> &_task)
{
    return _output << "Move " << ObjectTagMeta <ObjectTag>::Name << " \"" << _task.sourceName <<
                   "\" to \"" << _task.targetName << "\".";
}

template <typename ObjectTag>
std::ostream &operator << (std::ostream &_output, const Tasks::Copy <ObjectTag> &_task)
{
    return _output << "Copy " << ObjectTagMeta <ObjectTag>::Name << " \"" << _task.sourceName <<
                   "\" to \"" << _task.targetName << "\".";
}

template <typename ObjectTag>
std::ostream &operator << (std::ostream &_output, const Tasks::MoveAssign <ObjectTag> &_task)
{
    return _output << "Move " << ObjectTagMeta <ObjectTag>::Name << " \"" << _task.sourceName << "\" to \"" <<
                   _task.targetName << "\" using move assignment.";
}

template <typename ObjectTag>
std::ostream &operator << (std::ostream &_output, const Tasks::CopyAssign <ObjectTag> &_task)
{
    return _output << "Assign copy of " << ObjectTagMeta <ObjectTag>::Name << " \"" <<
                   _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

template <typename ObjectTag>
std::ostream &operator << (std::ostream &_output, const Tasks::Delete <ObjectTag> &_task)
{
    return _output << "Remove " << ObjectTagMeta <ObjectTag>::Name << " \"" << _task.name << "\".";
}
} // namespace Tasks
} // namespace Emergence::Context::Extension

#define EMERGENCE_CONTEXT_BIND_OBJECT_TAG(ObjectTag, ObjectType, ReadableName) \
namespace Emergence::Context::Extension                                        \
{                                                                              \
template <>                                                                    \
struct ObjectTagMeta <ObjectTag>                                               \
{                                                                              \
    using Object = ObjectType;                                                 \
    static constexpr const char *Name = ReadableName;                          \
};                                                                             \
} // namespace Emergence::Context::Extension