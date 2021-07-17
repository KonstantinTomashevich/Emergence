#include <Query/Test/DataTypes.hpp>

#include <StandardLayout/MappingBuilder.hpp>

namespace Emergence::Query::Test
{
static StandardLayout::Mapping RegisterPlayer ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (Player));

    Player::Reflection::id = builder.RegisterUInt32 (offsetof (Player, id));
    Player::Reflection::name = builder.RegisterString (offsetof (Player, name), Player::NAME_MAX_SIZE);

    Player::Reflection::alive = builder.RegisterBit (offsetof (Player, status), Player::Status::FLAG_ALIVE_OFFSET);
    Player::Reflection::stunned = builder.RegisterBit (offsetof (Player, status), Player::Status::FLAG_STUNNED_OFFSET);

    Player::Reflection::poisoned = builder.RegisterBit (
        offsetof (Player, status), Player::Status::FLAG_POISONED_OFFSET);

    Player::Reflection::immobilized = builder.RegisterBit (
        offsetof (Player, status), Player::Status::FLAG_IMMOBILIZED_OFFSET);
    return builder.End ();
}

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

static StandardLayout::Mapping RegisterScreenRect ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (ScreenRect));

    ScreenRect::Reflection::minX = builder.RegisterInt16 (offsetof (ScreenRect, minX));
    ScreenRect::Reflection::minY = builder.RegisterInt16 (offsetof (ScreenRect, minY));

    ScreenRect::Reflection::maxX = builder.RegisterInt16 (offsetof (ScreenRect, maxX));
    ScreenRect::Reflection::maxY = builder.RegisterInt16 (offsetof (ScreenRect, maxY));
    return builder.End ();
}

static StandardLayout::Mapping RegisterMergedRecord ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (MergedRecord));

    MergedRecord::Reflection::player = builder.RegisterNestedObject (
        offsetof (MergedRecord, player), Player::Reflection::GetMapping ());

    MergedRecord::Reflection::boundingBox = builder.RegisterNestedObject (
        offsetof (MergedRecord, boundingBox), BoundingBox::Reflection::GetMapping ());

    MergedRecord::Reflection::screenRect = builder.RegisterNestedObject (
        offsetof (MergedRecord, screenRect), ScreenRect::Reflection::GetMapping ());
    return builder.End ();
}

static StandardLayout::Mapping RegisterAllFieldTypesStructure ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin (sizeof (AllFieldTypesStructure));

    AllFieldTypesStructure::Reflection::int8 = builder.RegisterInt8 (offsetof (AllFieldTypesStructure, int8));
    AllFieldTypesStructure::Reflection::int16 = builder.RegisterInt16 (offsetof (AllFieldTypesStructure, int16));
    AllFieldTypesStructure::Reflection::int32 = builder.RegisterInt32 (offsetof (AllFieldTypesStructure, int32));
    AllFieldTypesStructure::Reflection::int64 = builder.RegisterInt64 (offsetof (AllFieldTypesStructure, int64));

    AllFieldTypesStructure::Reflection::uint8 = builder.RegisterUInt8 (offsetof (AllFieldTypesStructure, uint8));
    AllFieldTypesStructure::Reflection::uint16 = builder.RegisterUInt16 (offsetof (AllFieldTypesStructure, uint16));
    AllFieldTypesStructure::Reflection::uint32 = builder.RegisterUInt32 (offsetof (AllFieldTypesStructure, uint32));
    AllFieldTypesStructure::Reflection::uint64 = builder.RegisterUInt64 (offsetof (AllFieldTypesStructure, uint64));

    AllFieldTypesStructure::Reflection::floating = builder.RegisterFloat (offsetof (AllFieldTypesStructure, floating));
    AllFieldTypesStructure::Reflection::doubleFloating =
        builder.RegisterDouble (offsetof (AllFieldTypesStructure, doubleFloating));

    AllFieldTypesStructure::Reflection::block = builder.RegisterBlock (
        offsetof (AllFieldTypesStructure, block), sizeof (AllFieldTypesStructure::block));

    AllFieldTypesStructure::Reflection::string = builder.RegisterString (
        offsetof (AllFieldTypesStructure, string), sizeof (AllFieldTypesStructure::string));
    return builder.End ();
}

StandardLayout::FieldId Player::Reflection::id;

StandardLayout::FieldId Player::Reflection::name;

StandardLayout::FieldId Player::Reflection::alive;

StandardLayout::FieldId Player::Reflection::stunned;

StandardLayout::FieldId Player::Reflection::poisoned;

StandardLayout::FieldId Player::Reflection::immobilized;

StandardLayout::FieldId BoundingBox::Reflection::minX;

StandardLayout::FieldId BoundingBox::Reflection::minY;

StandardLayout::FieldId BoundingBox::Reflection::minZ;

StandardLayout::FieldId BoundingBox::Reflection::maxX;

StandardLayout::FieldId BoundingBox::Reflection::maxY;

StandardLayout::FieldId BoundingBox::Reflection::maxZ;

StandardLayout::FieldId ScreenRect::Reflection::minX;

StandardLayout::FieldId ScreenRect::Reflection::minY;

StandardLayout::FieldId ScreenRect::Reflection::maxX;

StandardLayout::FieldId ScreenRect::Reflection::maxY;

StandardLayout::FieldId MergedRecord::Reflection::player;

StandardLayout::FieldId MergedRecord::Reflection::boundingBox;

StandardLayout::FieldId MergedRecord::Reflection::screenRect;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::int8;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::int16;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::int32;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::int64;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::uint8;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::uint16;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::uint32;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::uint64;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::floating;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::doubleFloating;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::block;

StandardLayout::FieldId AllFieldTypesStructure::Reflection::string;

StandardLayout::Mapping Player::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterPlayer ();
    return mapping;
}

StandardLayout::Mapping BoundingBox::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterBoundingBox ();
    return mapping;
}

StandardLayout::Mapping ScreenRect::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterScreenRect ();
    return mapping;
}

StandardLayout::Mapping MergedRecord::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterMergedRecord ();
    return mapping;
}

StandardLayout::Mapping AllFieldTypesStructure::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterAllFieldTypesStructure ();
    return mapping;
}
} // namespace Query::Test