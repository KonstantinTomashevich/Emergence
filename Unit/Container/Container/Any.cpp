#include <Container/Any.hpp>
#include <utility>

namespace Emergence::Container
{
Any::Any (Memory::Profiler::AllocationGroup _group) noexcept
    : heap (std::move (_group))
{
}

Any::Any (Any &&_other) noexcept
    : pointer (_other.pointer),
      size (_other.size),
      heap (std::move (_other.heap))
{
    _other.pointer = nullptr;
}

Any::~Any () noexcept
{
    Reset ();
}

const void *Any::Get () const noexcept
{
    return pointer;
}

void *Any::Get () noexcept
{
    return pointer;
}

void Any::Reset () noexcept
{
    if (pointer && destructor)
    {
        destructor (pointer);
        heap.Release (pointer, size);
    }

    pointer = nullptr;
    size = 0u;
    destructor = nullptr;
}

Any &Any::operator= (Any &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Any ();
        new (this) Any (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Container
