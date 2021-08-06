#pragma once

#include <vector>

#include <Galleon/SingletonContainer.hpp>
#include <Galleon/ShortTermContainer.hpp>
#include <Galleon/LongTermContainer.hpp>

#include <Handling/Handle.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Galleon
{
class CargoDeck final
{
public:
    CargoDeck () noexcept = default;

    /// CargoDeck manages lots of storages with lots of objects, therefore it's not optimal to copy it.
    CargoDeck (const CargoDeck &_other) = delete;

    CargoDeck (CargoDeck &&_other) noexcept;

    /// \invariant There is no handles to any container and no prepared queries, that belong this deck.
    ~CargoDeck () noexcept;

    Handling::Handle <SingletonContainer> AcquireSingletonContainer (const StandardLayout::Mapping &_typeMapping);

    Handling::Handle <ShortTermContainer> AcquireShortTermContainer (const StandardLayout::Mapping &_typeMapping);

    Handling::Handle <LongTermContainer> AcquireLongTermContainer (const StandardLayout::Mapping &_typeMapping);

    /// CargoDeck manages lots of storages with lots of objects, therefore it's not optimal to copy assign it.
    CargoDeck &operator = (const CargoDeck &_other) = delete;

    CargoDeck &operator = (CargoDeck &&_other) noexcept;

private:
    friend class SingletonContainer;

    friend class ShortTermContainer;

    friend class LongTermContainer;

    void DetachContainer (SingletonContainer *_container) noexcept;

    void DetachContainer (ShortTermContainer *_container) noexcept;

    void DetachContainer (LongTermContainer *_container) noexcept;

    struct
    {
        std::vector <SingletonContainer *> singleton {};
        std::vector <ShortTermContainer *> shortTerm {};
        std::vector <LongTermContainer *> longTerm {};
    } containers {};
};
} // namespace Emergence::Galleon