#pragma once

#include <StandardLayout/Mapping.hpp>

#include <Warehouse/SingletonStorage.hpp>
#include <Warehouse/ShortTermStorage.hpp>
#include <Warehouse/LongTermStorage.hpp>

namespace Emergence::Warehouse
{
class Registry final
{
public:
    Registry () noexcept;

    ~Registry ();

    SingletonStorage AcquireSingletonStorage (const StandardLayout::Mapping _typeMapping) noexcept;

    ShortTermStorage AcquireShortTermStorage (const StandardLayout::Mapping _typeMapping) noexcept;

    LongTermStorage AcquireLongTermStorage (const StandardLayout::Mapping _typeMapping) noexcept;

private:
    void *handle;
};
} // namespace Emergence::Warehouse