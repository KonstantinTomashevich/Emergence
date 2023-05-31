#pragma once

#include <array>
#include <cstdint>
#include <cstdlib>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <StandardLayout/Field.hpp>
#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>

namespace Emergence::StandardLayout
{
constexpr std::size_t VALUE_MAX_SIZE = sizeof (std::uint64_t);

struct ValueSetter
{
    FieldId field;
    std::array<std::uint8_t, VALUE_MAX_SIZE> value;
};

static_assert (std::is_trivial_v<ValueSetter>);

class PlainPatch final : public Handling::HandleableBase
{
public:
    PlainPatch (const PlainPatch &_other) = delete;

    PlainPatch (PlainPatch &&_other) = delete;

    Mapping GetTypeMapping () const noexcept;

    void Apply (void *_object) const noexcept;

    std::size_t GetValueCount () const noexcept;

    const ValueSetter *Begin () const noexcept;

    const ValueSetter *End () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PlainPatch);

private:
    /// PlainPatchBuilder constructs PlainPatch's.
    friend class PlainPatchBuilder;

    /// Handle calls PlainPatch destructor.
    template <typename>
    friend class Handling::Handle;

    /// \return Heap, used for PlainPatch allocation.
    static Memory::Heap &GetHeap () noexcept;

    /// \return Size of patch object, that can hold up to _valueCapacity values.
    static std::size_t CalculatePatchSize (std::size_t _valueCapacity) noexcept;

    explicit PlainPatch (Mapping _mapping) noexcept;

    ~PlainPatch () noexcept = default;

    /// \brief Allocates patch object, that can hold up to _valueCapacity values.
    ///
    /// \details PlainPatch uses malloc-based allocation to support runtime capacity changes using ::ChangeCapacity.
    void *operator new (std::size_t /*unused*/, std::size_t _valueCapacity) noexcept;

    /// \brief PlainPatch has custom allocation logic and therefore needs custom deallocator.
    void operator delete (void *_pointer) noexcept;

    /// \brief Changes patch object size by changing its value capacity.
    /// \invariant ::valueCount is less or equal to _newValueCapacity.
    ///
    /// \warning This operation can cause reallocation of patch object.
    ///          Always use returned pointer instead of old one.
    /// \return Pointer to new location of this patch object.
    PlainPatch *ChangeCapacity (std::size_t _newValueCapacity) noexcept;

    Mapping mapping;
    std::size_t valueCount = 0u;
    std::size_t valueCapacity = 0u;
    ValueSetter valueSetters[0u];
};

class PlainPatchBuilder final
{
public:
    PlainPatchBuilder () noexcept = default;

    PlainPatchBuilder (const PlainPatchBuilder &_other) = delete;

    PlainPatchBuilder (PlainPatchBuilder &&_other) noexcept;

    ~PlainPatchBuilder () noexcept;

    void Begin (Mapping _mapping) noexcept;

    void Set (FieldId _field, const std::array<std::uint8_t, VALUE_MAX_SIZE> &_value) noexcept;

    PlainPatch *End () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PlainPatchBuilder);

private:
    static constexpr std::size_t INITIAL_VALUE_CAPACITY = 16u;

    PlainPatch *underConstruction = nullptr;
};
} // namespace Emergence::StandardLayout
