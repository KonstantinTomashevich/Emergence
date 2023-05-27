#pragma once

#include <API/Common/Cursor.hpp>

#include <Container/Vector.hpp>

#include <Pegasus/IndexBase.hpp>

namespace Emergence::Pegasus
{
class SignalIndex final : public IndexBase
{
public:
    class ReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (ReadCursor);

    private:
        friend class SignalIndex;

        ReadCursor (SignalIndex *_index) noexcept;

        SignalIndex *index;
        Container::Vector<const void *>::const_iterator current;
    };

    class EditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (EditCursor);

    private:
        friend class SignalIndex;

        EditCursor (SignalIndex *_index) noexcept;

        void BeginRecordEdition () const noexcept;

        SignalIndex *index;
        Container::Vector<const void *>::iterator current;
    };

    /// There is no sense to copy indices.
    SignalIndex (const SignalIndex &_other) = delete;

    /// Moving indices is forbidden, because otherwise user can move index out of Storage.
    SignalIndex (SignalIndex &&_other) = delete;

    ReadCursor LookupSignaledToRead ();

    EditCursor LookupSignaledToEdit ();

    [[nodiscard]] const StandardLayout::Field &GetIndexedField () const;

    [[nodiscard]] bool IsSignaledValue (const std::array<std::uint8_t, sizeof (std::uint64_t)> &_value) const;

    [[nodiscard]] std::array<std::uint8_t, sizeof (std::uint64_t)> GetSignaledValue () const;

    void Drop () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (SignalIndex);

private:
    friend class Storage;

    SignalIndex (Storage *_owner,
                 StandardLayout::FieldId _indexedField,
                 const std::array<std::uint8_t, sizeof (std::uint64_t)> &_signaledValue) noexcept;

    ~SignalIndex () noexcept = default;

    bool IsSignaled (const void *_record) const noexcept;

    void InsertRecord (const void *_record) noexcept;

    void OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept;

    void DeleteRecordMyself (const Container::Vector<const void *>::iterator &_position) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    void OnRecordChangedByMe (Container::Vector<const void *>::iterator _position) noexcept;

    void OnWriterClosed () noexcept;

    StandardLayout::Field indexedField;
    const std::size_t offset;
    const std::uint64_t mask;
    const std::uint64_t signaledValue;
    Container::Vector<const void *> signaledRecords;
};
} // namespace Emergence::Pegasus
