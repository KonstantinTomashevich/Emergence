#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

namespace Emergence::Render::Backend
{
/// \brief Temporary index buffer that will be automatically destructed after frame is rendered.
/// \details Useful for drawing geometry that might change every frame. For example, batched 2d sprites or UI.
///          Must be thread safe.
class TransientIndexBuffer
{
public:
    /// \brief If there is not enough space to allocate buffer for required count, returns maximum possible count.
    static std::uint32_t TruncateSizeToAvailability (std::uint32_t _indexCount, bool _use32BitIndices);

    /// \brief Constructs temporary buffer for given count of indices using 32-bit indices if requested.
    TransientIndexBuffer (std::uint32_t _indexCount, bool _use32BitIndices) noexcept;

    TransientIndexBuffer (const TransientIndexBuffer &_other) = delete;

    TransientIndexBuffer (TransientIndexBuffer &&_other) = delete;

    ~TransientIndexBuffer () noexcept; // NOLINT(performance-trivially-destructible): Implementation-specific.

    /// \return Pointer to buffer data.
    void *GetData () noexcept;

    /// \return Pointer to buffer data.
    [[nodiscard]] const void *GetData () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (TransientIndexBuffer);

private:
    friend class SubmissionAgent;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 3u);
};
} // namespace Emergence::Render::Backend
