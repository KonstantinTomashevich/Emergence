#include <Pegasus/Test/Record.hpp>

#include <StandardLayout/MappingBuilder.hpp>

namespace Emergence::Pegasus::Test
{
static StandardLayout::Mapping RegisterBoundingBox ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (BoundingBox));

    BoundingBox::Reflection::minX = builder.RegisterFloat (offsetof (BoundingBox, minX));
    BoundingBox::Reflection::minY = builder.RegisterFloat (offsetof (BoundingBox, minY));
    BoundingBox::Reflection::minZ = builder.RegisterFloat (offsetof (BoundingBox, minZ));

    BoundingBox::Reflection::maxX = builder.RegisterFloat (offsetof (BoundingBox, maxX));
    BoundingBox::Reflection::maxY = builder.RegisterFloat (offsetof (BoundingBox, maxY));
    BoundingBox::Reflection::maxZ = builder.RegisterFloat (offsetof (BoundingBox, maxZ));
    return builder.End ();
}

static StandardLayout::Mapping RegisterRecord ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (Record));

    Record::Reflection::entityId = builder.RegisterUInt32 (offsetof (Record, entityId));
    Record::Reflection::nickname = builder.RegisterString (offsetof (Record, nickname), Record::NICKNAME_MAX_SIZE);
    Record::Reflection::boundingBox = builder.RegisterNestedObject (
        offsetof (Record, boundingBox), BoundingBox::Reflection::GetMapping ());

    Record::Reflection::alive = builder.RegisterBit (offsetof (Record, status), Record::Status::FLAG_ALIVE_OFFSET);
    Record::Reflection::stunned = builder.RegisterBit (offsetof (Record, status), Record::Status::FLAG_STUNNED_OFFSET);

    Record::Reflection::poisoned = builder.RegisterBit (
        offsetof (Record, status), Record::Status::FLAG_POISONED_OFFSET);

    Record::Reflection::immobilized = builder.RegisterBit (
        offsetof (Record, status), Record::Status::FLAG_IMMOBILIZED_OFFSET);
    return builder.End ();
}

StandardLayout::FieldId BoundingBox::Reflection::minX;

StandardLayout::FieldId BoundingBox::Reflection::minY;

StandardLayout::FieldId BoundingBox::Reflection::minZ;

StandardLayout::FieldId BoundingBox::Reflection::maxX;

StandardLayout::FieldId BoundingBox::Reflection::maxY;

StandardLayout::FieldId BoundingBox::Reflection::maxZ;

StandardLayout::FieldId Record::Reflection::entityId;

StandardLayout::FieldId Record::Reflection::nickname;

StandardLayout::FieldId Record::Reflection::boundingBox;

StandardLayout::FieldId Record::Reflection::alive;

StandardLayout::FieldId Record::Reflection::stunned;

StandardLayout::FieldId Record::Reflection::poisoned;

StandardLayout::FieldId Record::Reflection::immobilized;

StandardLayout::Mapping BoundingBox::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterBoundingBox ();
    return mapping;
}

StandardLayout::Mapping Record::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterRecord ();
    return mapping;
}
} // namespace Emergence::Pegasus::Test
