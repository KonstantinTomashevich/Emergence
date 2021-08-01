#include <cassert>

#include <Galleon/ContainerBase.hpp>

namespace Emergence::Galleon
{
const StandardLayout::Mapping &ContainerBase::GetTypeMapping () const noexcept
{
    return typeMapping;
}

ContainerBase::ContainerBase (class CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : deck (_deck),
      typeMapping (std::move (_typeMapping)),
      readCursorCount (0u),
      modificationCursorCount (0u)
{
    assert (deck);
}

void ContainerBase::RegisterReadCursor () noexcept
{
    // Modification cursor count can not be changed by thread safe operations, therefore it's ok to check it here.
    assert (modificationCursorCount == 0u);
    ++readCursorCount;
}

void ContainerBase::UnregisterReadCursor () noexcept
{
    --readCursorCount;
}

void ContainerBase::RegisterModificationCursor () noexcept
{
    assert (readCursorCount == 0u);
    assert (modificationCursorCount == 0u);
    ++modificationCursorCount;
}

void ContainerBase::UnregisterModificationCursor () noexcept
{
    assert (modificationCursorCount == 1u);
    --modificationCursorCount;
}
} // namespace Emergence::Galleon
