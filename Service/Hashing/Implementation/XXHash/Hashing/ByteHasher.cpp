#include <cassert>
#include <xxhash.hpp>

#include <Hashing/ByteHasher.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Hashing
{
ByteHasher::ByteHasher () noexcept
{
    new (&data) xxh::hash_state64_t ();
}

ByteHasher::~ByteHasher () noexcept
{
    block_cast<xxh::hash_state64_t> (data).~hash_state_t ();
}

void ByteHasher::Append (uint8_t _byte) noexcept
{
    block_cast<xxh::hash_state64_t> (data).update (&_byte, 1u);
}

void ByteHasher::Append (const uint8_t *_bytes, std::size_t _count) noexcept
{
    assert (_bytes);
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
} // namespace Emergence::Hashing
