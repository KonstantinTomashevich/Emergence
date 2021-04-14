#include <cstdlib>
#include <new>

#include <StandardLayout/Original/PlainMapping.hpp>

namespace Emergence::StandardLayout
{
std::size_t CalculateMappingSize (std::size_t _fieldCapacity)
{
    return sizeof (PlainMapping) /*+ _fieldCapacity * sizeof (FieldMeta) */;
}

PlainMapping *PlainMapping::Allocate (std::size_t _fieldCapacity) noexcept
{
    return new (malloc (CalculateMappingSize (_fieldCapacity))) PlainMapping ();
}

PlainMapping *PlainMapping::Reallocate (std::size_t _fieldCapacity) noexcept
{
    return static_cast <PlainMapping *> (realloc (this, CalculateMappingSize (_fieldCapacity)));
}

//const FieldMeta *PlainMapping::GetField (FieldId _field) const noexcept
//{
//    return reinterpret_cast <const FieldMeta *> (this + 1u) + _field;
//}
//
//FieldMeta *PlainMapping::GetField (FieldId _field) noexcept
//{
//    return reinterpret_cast <FieldMeta *> (this + 1u) + _field;
//}
} // namespace Emergence::StandardLayout
