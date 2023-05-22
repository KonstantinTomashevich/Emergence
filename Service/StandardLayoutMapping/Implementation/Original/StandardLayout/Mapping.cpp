#include <API/Common/Implementation/Iterator.hpp>

#include <Assert/Assert.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
using FieldIterator = Mapping::FieldIterator;

using FieldIteratorImplementation = const FieldData *;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (FieldIterator, FieldIteratorImplementation)

Field Mapping::FieldIterator::operator* () const noexcept
{
    return Field (block_cast<FieldData *> (data));
}

using ConditionalFieldIterator = Mapping::ConditionalFieldIterator;

using ConditionalFieldIteratorImplementation = PlainMapping::ConditionalFieldIterator;

EMERGENCE_BIND_FORWARD_ITERATOR_OPERATIONS_IMPLEMENTATION (ConditionalFieldIterator,
                                                           ConditionalFieldIteratorImplementation)

Field Mapping::ConditionalFieldIterator::operator* () const noexcept
{
    return Field (const_cast<FieldData *> (*block_cast<PlainMapping::ConditionalFieldIterator> (data)));
}

Mapping::Mapping () noexcept
{
    new (&data) Handling::Handle<PlainMapping> (nullptr);
}

Mapping::Mapping (const Mapping &_other) noexcept
    : Mapping (_other.data)
{
}

Mapping::Mapping (Mapping &&_other) noexcept
    : Mapping (_other.data)
{
}

std::size_t Mapping::GetObjectSize () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    return handle->GetObjectSize ();
}

std::size_t Mapping::GetObjectAlignment () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    return handle->GetObjectAlignment ();
}

Memory::UniqueString Mapping::GetName () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    return handle->GetName ();
}

void Mapping::Construct (void *_address) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    handle->Construct (_address);
}

void Mapping::MoveConstruct (void *_address, void *_sourceAddress) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    handle->MoveConstruct (_address, _sourceAddress);
}

void Mapping::Destruct (void *_address) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    handle->Destruct (_address);
}

Field Mapping::GetField (FieldId _field) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    return Field (const_cast<FieldData *> (handle->GetField (_field)));
}

bool Mapping::operator== (const Mapping &_other) const noexcept
{
    return block_cast<Handling::Handle<PlainMapping>> (data) ==
           block_cast<Handling::Handle<PlainMapping>> (_other.data);
}

bool Mapping::operator!= (const Mapping &_other) const noexcept
{
    return !(*this == _other);
}

Mapping &Mapping::operator= (const Mapping &_other) noexcept
{
    if (this != &_other)
    {
        this->~Mapping ();
        new (this) Mapping (_other);
    }

    return *this;
}

Mapping &Mapping::operator= (Mapping &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Mapping ();
        new (this) Mapping (std::move (_other));
    }

    return *this;
}

Mapping::FieldIterator Mapping::Begin () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    const FieldData *iterator = handle->Begin ();
    return FieldIterator (array_cast (iterator));
}

Mapping::FieldIterator Mapping::End () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    const FieldData *iterator = handle->End ();
    return FieldIterator (array_cast (iterator));
}

ConditionalFieldIterator Mapping::BeginConditional (const void *_object) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    const PlainMapping::ConditionalFieldIterator iterator = handle->BeginConditional (_object);
    return ConditionalFieldIterator (array_cast (iterator));
}

ConditionalFieldIterator Mapping::EndConditional () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    const PlainMapping::ConditionalFieldIterator iterator = handle->EndConditional ();
    return ConditionalFieldIterator (array_cast (iterator));
}

FieldId Mapping::GetFieldId (const Mapping::FieldIterator &_iterator) const noexcept
{
    return GetFieldId (*_iterator);
}

FieldId Mapping::GetFieldId (const Field &_field) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    EMERGENCE_ASSERT (handle);
    return handle->GetFieldId (*reinterpret_cast<const FieldData *> (_field.handle));
}

uintptr_t Mapping::Hash () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    return reinterpret_cast<uintptr_t> (handle.Get ());
}

bool Mapping::IsHandleValid () const noexcept
{
    return block_cast<Handling::Handle<PlainMapping>> (data).Get ();
}

Mapping::operator bool () const noexcept
{
    return IsHandleValid ();
}

Mapping::Mapping (const std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Handling::Handle<PlainMapping> (block_cast<Handling::Handle<PlainMapping>> (_data));
}

Mapping::Mapping (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Handling::Handle<PlainMapping> (std::move (block_cast<Handling::Handle<PlainMapping>> (_data)));
}

Mapping::~Mapping () noexcept
{
    block_cast<Handling::Handle<PlainMapping>> (data).~Handle ();
}

Mapping::FieldIterator begin (const Mapping &_mapping) noexcept
{
    return _mapping.Begin ();
}

Mapping::FieldIterator end (const Mapping &_mapping) noexcept
{
    return _mapping.End ();
}
} // namespace Emergence::StandardLayout
