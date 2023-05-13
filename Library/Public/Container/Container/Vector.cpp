#include <Assert/Assert.hpp>

#include <Container/Vector.hpp>

namespace Emergence::Container::UntypedVectorUtility
{
void InitSize (void *_vector, size_t _sizeInBytes) noexcept
{
    EMERGENCE_ASSERT (static_cast<Container::Vector<uint8_t> *> (_vector)->empty ());
    static_cast<Container::Vector<uint8_t> *> (_vector)->resize (_sizeInBytes);
}

uint8_t *Begin (void *_vector) noexcept
{
    return &*static_cast<Container::Vector<uint8_t> *> (_vector)->begin ();
}

uint8_t *End (void *_vector) noexcept
{
    return &*static_cast<Container::Vector<uint8_t> *> (_vector)->end ();
}

const uint8_t *Begin (const void *_vector) noexcept
{
    return &*static_cast<const Container::Vector<uint8_t> *> (_vector)->begin ();
}

const uint8_t *End (const void *_vector) noexcept
{
    return &*static_cast<const Container::Vector<uint8_t> *> (_vector)->end ();
}
} // namespace Emergence::Container::UntypedVectorUtility
