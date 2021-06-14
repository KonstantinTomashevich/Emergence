#include <Pegasus/Test/Record.hpp>

#include <StandardLayout/MappingBuilder.hpp>

namespace Emergence::Pegasus::Test
{
static StandardLayout::Mapping RegisterAvatarInfo ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (AvatarInfo));

    AvatarInfo::Reflection::teamId = builder.RegisterInt8 (offsetof (AvatarInfo, teamId));
    AvatarInfo::Reflection::classId = builder.RegisterInt8 (offsetof (AvatarInfo, classId));
    return builder.End ();
}

static StandardLayout::Mapping RegisterAvatarRuntime ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (AvatarRuntime));

    AvatarRuntime::Reflection::ammoLeft = builder.RegisterInt16 (offsetof (AvatarRuntime, ammoLeft));
    AvatarRuntime::Reflection::health = builder.RegisterFloat (offsetof (AvatarRuntime, health));
    return builder.End ();
}

static StandardLayout::Mapping RegisterTransform ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (Transform));

    Transform::Reflection::x = builder.RegisterFloat (offsetof (Transform, x));
    Transform::Reflection::y = builder.RegisterFloat (offsetof (Transform, y));
    Transform::Reflection::rotationDeg = builder.RegisterFloat (offsetof (Transform, rotationDeg));
    return builder.End ();
}

static StandardLayout::Mapping RegisterRecord ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (Record));

    Record::Reflection::entityId = builder.RegisterUInt32 (offsetof (Record, entityId));
    Record::Reflection::nickname = builder.RegisterString (offsetof (Record, nickname), Record::NICKNAME_MAX_SIZE);

    Record::Reflection::info = builder.RegisterNestedObject (
        offsetof (Record, info), AvatarInfo::Reflection::GetMapping ());

    Record::Reflection::runtime = builder.RegisterNestedObject (
        offsetof (Record, runtime), AvatarRuntime::Reflection::GetMapping ());

    Record::Reflection::transform = builder.RegisterNestedObject (
        offsetof (Record, transform), Transform::Reflection::GetMapping ());

    Record::Reflection::alive = builder.RegisterBit (offsetof (Record, status), Record::Status::FLAG_ALIVE_OFFSET);
    Record::Reflection::stunned = builder.RegisterBit (offsetof (Record, status), Record::Status::FLAG_STUNNED_OFFSET);

    Record::Reflection::poisoned = builder.RegisterBit (
        offsetof (Record, status), Record::Status::FLAG_POISONED_OFFSET);

    Record::Reflection::immobilized = builder.RegisterBit (
        offsetof (Record, status), Record::Status::FLAG_IMMOBILIZED_OFFSET);
    return builder.End ();
}

StandardLayout::FieldId AvatarInfo::Reflection::teamId;

StandardLayout::FieldId AvatarInfo::Reflection::classId;

StandardLayout::FieldId AvatarRuntime::Reflection::ammoLeft;

StandardLayout::FieldId AvatarRuntime::Reflection::health;

StandardLayout::FieldId Transform::Reflection::x;

StandardLayout::FieldId Transform::Reflection::y;

StandardLayout::FieldId Transform::Reflection::rotationDeg;

StandardLayout::FieldId Record::Reflection::entityId;

StandardLayout::FieldId Record::Reflection::nickname;

StandardLayout::FieldId Record::Reflection::info;

StandardLayout::FieldId Record::Reflection::runtime;

StandardLayout::FieldId Record::Reflection::transform;

StandardLayout::FieldId Record::Reflection::alive;

StandardLayout::FieldId Record::Reflection::stunned;

StandardLayout::FieldId Record::Reflection::poisoned;

StandardLayout::FieldId Record::Reflection::immobilized;

StandardLayout::Mapping AvatarInfo::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterAvatarInfo ();
    return mapping;
}

StandardLayout::Mapping AvatarRuntime::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterAvatarRuntime ();
    return mapping;
}

StandardLayout::Mapping Transform::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterTransform ();
    return mapping;
}

StandardLayout::Mapping Record::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterRecord ();
    return mapping;
}
} // namespace Emergence::Pegasus::Test
