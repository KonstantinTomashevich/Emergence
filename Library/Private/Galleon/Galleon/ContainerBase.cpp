#include <Assert/Assert.hpp>

#include <Galleon/ContainerBase.hpp>

namespace Emergence::Galleon
{
const StandardLayout::Mapping &ContainerBase::GetTypeMapping () const noexcept
{
    return typeMapping;
}

ContainerBase::ContainerBase (CargoDeck *_deck, StandardLayout::Mapping _typeMapping) noexcept
    : deck (_deck),
      typeMapping (std::move (_typeMapping))
{
    EMERGENCE_ASSERT (deck);
}
} // namespace Emergence::Galleon
