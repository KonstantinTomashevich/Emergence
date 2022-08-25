#include <Asset/Object/Library.hpp>

namespace Emergence::Asset::Object
{
const Library::ObjectData *Library::Find (const Memory::UniqueString &_name) const noexcept
{
    if (auto iterator = objects.find (_name); iterator != objects.end ())
    {
        return &iterator->second;
    }

    return nullptr;
}

const Library::ObjectMap &Library::GetRegisteredObjectMap () const noexcept
{
    return objects;
}
} // namespace Emergence::Asset::Object
