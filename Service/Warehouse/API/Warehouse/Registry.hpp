#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <StandardLayout/Mapping.hpp>

#include <Warehouse/SingletonStorage.hpp>
#include <Warehouse/ShortTermStorage.hpp>
#include <Warehouse/LongTermStorage.hpp>

namespace Emergence::Warehouse
{
/// \brief Manages storages for different object types.
/// \details Registry serves as root node for data storages and allows user to create and access storages.
///          Registry object is unique-ownership handle for implementation instance.
/// \warning Registry operations could be quite slow and should not be called often.
class Registry final
{
public:
    Registry () noexcept;

    /// Registry manages lots of storages with lots of records, therefore it's not optimal to copy it.
    Registry (const Registry &_other) = delete;

    Registry (Registry &&_other) noexcept;

    ~Registry () noexcept;

    /// \return SingletonStorage for given type.
    /// \details If there is no existing storage for this type, new storage will be allocated.
    SingletonStorage AcquireSingletonStorage (const StandardLayout::Mapping _typeMapping) noexcept;

    /// \return ShortTermStorage for given type.
    /// \details If there is no existing storage for this type, new storage will be allocated.
    ShortTermStorage AcquireShortTermStorage (const StandardLayout::Mapping _typeMapping) noexcept;

    /// \return LongTermStorage for given type.
    /// \details If there is no existing storage for this type, new storage will be allocated.
    LongTermStorage AcquireLongTermStorage (const StandardLayout::Mapping _typeMapping) noexcept;

    /// Registry manages lots of storages with lots of records, therefore it's not optimal to copy assign it.
    Registry &operator = (const Registry &_other) = delete;

    Registry &operator = (Registry &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Warehouse