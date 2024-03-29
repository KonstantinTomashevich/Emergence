#include <API/Common/MuteWarnings.hpp>

BEGIN_MUTING_WARNINGS
#include <xxhash.hpp>
END_MUTING_WARNINGS

#include <API/Common/BlockCast.hpp>

#include <Assert/Assert.hpp>

#include <Hashing/ByteHasher.hpp>

namespace Emergence::Hashing
{
ByteHasher::ByteHasher () noexcept
{
    new (&data) xxh::hash_state64_t ();
}

ByteHasher::ByteHasher (const ByteHasher &_other) noexcept
{
    BEGIN_MUTING_WARNINGS
    new (&data) xxh::hash_state64_t (block_cast<xxh::hash_state64_t> (_other.data));
    END_MUTING_WARNINGS
}

ByteHasher::ByteHasher (ByteHasher &&_other) noexcept
{
    BEGIN_MUTING_WARNINGS
    new (&data) xxh::hash_state64_t (block_cast<xxh::hash_state64_t> (_other.data));
    END_MUTING_WARNINGS
}

ByteHasher::~ByteHasher () noexcept
{
    block_cast<xxh::hash_state64_t> (data).~hash_state_t ();
}

void ByteHasher::Append (std::uint8_t _byte) noexcept
{
    block_cast<xxh::hash_state64_t> (data).update (&_byte, 1u);
}

void ByteHasher::Append (const std::uint8_t *_bytes, std::size_t _count) noexcept
{
    EMERGENCE_ASSERT (_bytes);
    block_cast<xxh::hash_state64_t> (data).update (_bytes, _count);
}

void ByteHasher::Clear () noexcept
{
    block_cast<xxh::hash_state64_t> (data).reset ();
}

uint64_t ByteHasher::GetCurrentValue () const noexcept
{
    return block_cast<xxh::hash_state64_t> (data).digest ();
}

ByteHasher &ByteHasher::operator= (const ByteHasher &_other) noexcept
{
    if (this != &_other)
    {
        this->~ByteHasher ();
        new (this) ByteHasher (_other);
    }

    return *this;
}
ByteHasher &ByteHasher::operator= (ByteHasher &&_other) noexcept
{
    if (this != &_other)
    {
        this->~ByteHasher ();
        new (this) ByteHasher (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Hashing
