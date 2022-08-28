#pragma once

#include <Asset/Object/Object.hpp>

namespace Emergence::Asset::Object
{
/// \brief Storage for loaded objects.
/// \details Should be created using LibraryLoader.
class Library final
{
public:
    /// \brief Holds all known information about one particular object.
    struct ObjectData final
    {
        /// \brief Various information about this object.
        Declaration declaration;

        /// \brief This object content.
        Body body;

        /// \brief Whether this object was loaded because it is a dependency
        ///        of other object and would be ignored otherwise.
        /// \details When loading begins, user selects from which folders objects should be loaded.
        ///          But some objects might have outside dependencies that must be loaded too.
        ///          If object was loaded as such dependency, this flag will be `true` for it.
        bool loadedAsDependency = false;
    };

    /// \brief Map in which loaded objects are stored.
    using ObjectMap = Container::HashMap<Memory::UniqueString, ObjectData>;

    /// \return Data of object with given name or `nullptr` if there is no objects with such name.
    [[nodiscard]] const ObjectData *Find (const Memory::UniqueString &_name) const noexcept;

    /// \return Map with all loaded objects.
    [[nodiscard]] const ObjectMap &GetRegisteredObjectMap () const noexcept;

private:
    friend class LibraryLoader;

    /// \details Library can only be constructed by LibraryLoader.
    Library () noexcept = default;

    Container::HashMap<Memory::UniqueString, ObjectData> objects {
        Memory::Profiler::AllocationGroup {GetRootAllocationGroup (), Memory::UniqueString {"Library"}}};
};
} // namespace Emergence::Asset::Object
