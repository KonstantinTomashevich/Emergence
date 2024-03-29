#pragma once

#include <ResourceObjectApi.hpp>

#include <Resource/Object/Object.hpp>

namespace Emergence::Resource::Object
{
/// \brief Storage for loaded objects.
/// \details Should be created using LibraryLoader.
class ResourceObjectApi Library final
{
public:
    /// \brief Holds all known information about one particular object.
    struct ResourceObjectApi ObjectData final
    {
        /// \brief Content of this object.
        Object object;

        /// \brief Whether this object was loaded because it is a parent of any
        ///        of the requested objects and will not be loaded otherwise.
        /// \details When loading begins, user selects from which folders objects should be loaded.
        ///          But some objects might have outside objects as parents in inheritance.
        ///          If object was loaded as such parent, this flag will be `true` for it.
        bool loadedAsParent = false;
    };

    /// \brief Map in which loaded objects are stored.
    using ObjectMap = Container::HashMap<Memory::UniqueString, ObjectData>;

    /// \return Data of object with given name or `nullptr` if there is no objects with such name.
    [[nodiscard]] const ObjectData *Find (const Memory::UniqueString &_name) const noexcept;

    /// \return Map with all loaded objects.
    [[nodiscard]] const ObjectMap &GetRegisteredObjectMap () const noexcept;

private:
    friend class LibraryLoader;

    Container::HashMap<Memory::UniqueString, ObjectData> objects {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"Library"}}};
};
} // namespace Emergence::Resource::Object
