#pragma once

#include <Asset/Object/Object.hpp>

namespace Emergence::Asset::Object
{
class Library final
{
public:
    struct ObjectData final
    {
        Declaration declaration;
        Body body;
        bool loadedAsDependency = false;
    };

    using ObjectMap = Container::HashMap<Memory::UniqueString, ObjectData>;

    [[nodiscard]] const ObjectData *Find (const Memory::UniqueString &_name) const noexcept;

    [[nodiscard]] const ObjectMap &GetRegisteredObjectMap () const noexcept;

private:
    friend class LibraryLoader;

    Container::HashMap<Memory::UniqueString, ObjectData> objects {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"Library"}}};
};
} // namespace Emergence::Asset::Object