#include <Query/Test/DataTypes.hpp>

#include <StandardLayout/MappingBuilder.hpp>

namespace Emergence::Query::Test
{
static StandardLayout::Mapping RegisterPlayer ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin ("Player", sizeof (Player));

    Player::Reflection::id = builder.RegisterUInt32 ("id", offsetof (Player, id));
    Player::Reflection::name = builder.RegisterString ("name", offsetof (Player, name), Player::NAME_MAX_SIZE);

    Player::Reflection::alive =
        builder.RegisterBit ("alive", offsetof (Player, status), Player::Status::FLAG_ALIVE_OFFSET);
    Player::Reflection::stunned =
        builder.RegisterBit ("stunned", offsetof (Player, status), Player::Status::FLAG_STUNNED_OFFSET);

    Player::Reflection::poisoned =
        builder.RegisterBit ("poisoned", offsetof (Player, status), Player::Status::FLAG_POISONED_OFFSET);

    Player::Reflection::immobilized =
        builder.RegisterBit ("immobilized", offsetof (Player, status), Player::Status::FLAG_IMMOBILIZED_OFFSET);
    return builder.End ();
}

static StandardLayout::Mapping RegisterBoundingBox ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin ("BoundingBox", sizeof (BoundingBox));

    BoundingBox::Reflection::minX = builder.RegisterFloat ("minX", offsetof (BoundingBox, minX));
    BoundingBox::Reflection::minY = builder.RegisterFloat ("minY", offsetof (BoundingBox, minY));
    BoundingBox::Reflection::minZ = builder.RegisterFloat ("minZ", offsetof (BoundingBox, minZ));

    BoundingBox::Reflection::maxX = builder.RegisterFloat ("maxX", offsetof (BoundingBox, maxX));
    BoundingBox::Reflection::maxY = builder.RegisterFloat ("maxY", offsetof (BoundingBox, maxY));
    BoundingBox::Reflection::maxZ = builder.RegisterFloat ("maxZ", offsetof (BoundingBox, maxZ));
    return builder.End ();
}

static StandardLayout::Mapping RegisterScreenRect ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin ("ScreenRect", sizeof (ScreenRect));

    ScreenRect::Reflection::minX = builder.RegisterInt16 ("minX", offsetof (ScreenRect, minX));
    ScreenRect::Reflection::minY = builder.RegisterInt16 ("minY", offsetof (ScreenRect, minY));

    ScreenRect::Reflection::maxX = builder.RegisterInt16 ("maxX", offsetof (ScreenRect, maxX));
    ScreenRect::Reflection::maxY = builder.RegisterInt16 ("maxY", offsetof (ScreenRect, maxY));
    return builder.End ();
}

static StandardLayout::Mapping RegisterPlayerWithBoundingBox ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin ("PlayerWithBoundingBox", sizeof (PlayerWithBoundingBox));

    PlayerWithBoundingBox::Reflection::player = builder.RegisterNestedObject (
        "player", offsetof (PlayerWithBoundingBox, player), Player::Reflection::GetMapping ());

    PlayerWithBoundingBox::Reflection::boundingBox = builder.RegisterNestedObject (
        "boundingBox", offsetof (PlayerWithBoundingBox, boundingBox), BoundingBox::Reflection::GetMapping ());

    return builder.End ();
}

static StandardLayout::Mapping RegisterAllFieldTypesStructure ()
{
    StandardLayout::MappingBuilder builder;
    builder.Begin ("AllFieldTypesStructure", sizeof (AllFieldTypesStructure));

    AllFieldTypesStructure::Reflection::int8 = builder.RegisterInt8 ("int8", offsetof (AllFieldTypesStructure, int8));
    AllFieldTypesStructure::Reflection::int16 =
        builder.RegisterInt16 ("int16", offsetof (AllFieldTypesStructure, int16));
    AllFieldTypesStructure::Reflection::int32 =
        builder.RegisterInt32 ("int32", offsetof (AllFieldTypesStructure, int32));
    AllFieldTypesStructure::Reflection::int64 =
        builder.RegisterInt64 ("int64", offsetof (AllFieldTypesStructure, int64));

    AllFieldTypesStructure::Reflection::uint8 =
        builder.RegisterUInt8 ("uint8", offsetof (AllFieldTypesStructure, uint8));
    AllFieldTypesStructure::Reflection::uint16 =
        builder.RegisterUInt16 ("uint16", offsetof (AllFieldTypesStructure, uint16));
    AllFieldTypesStructure::Reflection::uint32 =
        builder.RegisterUInt32 ("uint32", offsetof (AllFieldTypesStructure, uint32));
    AllFieldTypesStructure::Reflection::uint64 =
        builder.RegisterUInt64 ("uint64", offsetof (AllFieldTypesStructure, uint64));

    AllFieldTypesStructure::Reflection::floating =
        builder.RegisterFloat ("floating", offsetof (AllFieldTypesStructure, floating));
    AllFieldTypesStructure::Reflection::doubleFloating =
        builder.RegisterDouble ("doubleFloating", offsetof (AllFieldTypesStructure, doubleFloating));

    AllFieldTypesStructure::Reflection::block = builder.RegisterBlock (
        "block", offsetof (AllFieldTypesStructure, block), sizeof (AllFieldTypesStructure::block));

    AllFieldTypesStructure::Reflection::string = builder.RegisterString (
        "string", offsetof (AllFieldTypesStructure, string), sizeof (AllFieldTypesStructure::string));
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

StandardLayout::FieldId PlayerWithBoundingBox::Reflection::player;

StandardLayout::FieldId PlayerWithBoundingBox::Reflection::boundingBox;

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

StandardLayout::Mapping PlayerWithBoundingBox::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterPlayerWithBoundingBox ();
    return mapping;
}

StandardLayout::Mapping AllFieldTypesStructure::Reflection::GetMapping ()
{
    static StandardLayout::Mapping mapping = RegisterAllFieldTypesStructure ();
    return mapping;
}
} // namespace Emergence::Query::Test
