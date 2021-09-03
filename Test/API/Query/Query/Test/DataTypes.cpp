#include <Query/Test/DataTypes.hpp>

#include <StandardLayout/MappingBuilder.hpp>

namespace Emergence::Query::Test
{
const Player::Reflection &Player::Reflect ()
{
    static auto reflection = [] () -> Reflection
    {
        StandardLayout::MappingBuilder builder;
        builder.Begin ("Player", sizeof (Player));

        return {
            .id = builder.RegisterUInt32 ("id", offsetof (Player, id)),
            .name = builder.RegisterString ("name", offsetof (Player, name), Player::NAME_MAX_SIZE),

            .alive = builder.RegisterBit ("alive", offsetof (Player, status), Player::Status::FLAG_ALIVE_OFFSET),
            .stunned = builder.RegisterBit ("stunned", offsetof (Player, status), Player::Status::FLAG_STUNNED_OFFSET),
            .poisoned =
                builder.RegisterBit ("poisoned", offsetof (Player, status), Player::Status::FLAG_POISONED_OFFSET),
            .immobilized =
                builder.RegisterBit ("immobilized", offsetof (Player, status), Player::Status::FLAG_IMMOBILIZED_OFFSET),

            .mapping = builder.End (),
        };
    }();

    return reflection;
}

const BoundingBox::Reflection &BoundingBox::Reflect ()
{
    static auto reflection = [] () -> Reflection
    {
        StandardLayout::MappingBuilder builder;
        builder.Begin ("BoundingBox", sizeof (BoundingBox));

        return {
            .minX = builder.RegisterFloat ("minX", offsetof (BoundingBox, minX)),
            .minY = builder.RegisterFloat ("minY", offsetof (BoundingBox, minY)),
            .minZ = builder.RegisterFloat ("minZ", offsetof (BoundingBox, minZ)),

            .maxX = builder.RegisterFloat ("maxX", offsetof (BoundingBox, maxX)),
            .maxY = builder.RegisterFloat ("maxY", offsetof (BoundingBox, maxY)),
            .maxZ = builder.RegisterFloat ("maxZ", offsetof (BoundingBox, maxZ)),

            .mapping = builder.End (),
        };
    }();

    return reflection;
}

const ScreenRect::Reflection &ScreenRect::Reflect ()
{
    static auto reflection = [] () -> Reflection
    {
        StandardLayout::MappingBuilder builder;
        builder.Begin ("ScreenRect", sizeof (ScreenRect));

        return {
            .minX = builder.RegisterInt16 ("minX", offsetof (ScreenRect, minX)),
            .minY = builder.RegisterInt16 ("minY", offsetof (ScreenRect, minY)),

            .maxX = builder.RegisterInt16 ("maxX", offsetof (ScreenRect, maxX)),
            .maxY = builder.RegisterInt16 ("maxY", offsetof (ScreenRect, maxY)),

            .mapping = builder.End (),
        };
    }();

    return reflection;
}

const PlayerWithBoundingBox::Reflection &PlayerWithBoundingBox::Reflect ()
{
    static auto reflection = [] () -> Reflection
    {
        StandardLayout::MappingBuilder builder;
        builder.Begin ("PlayerWithBoundingBox", sizeof (PlayerWithBoundingBox));

        return {
            .player = builder.RegisterNestedObject ("player", offsetof (PlayerWithBoundingBox, player),
                                                    Player::Reflect ().mapping),
            .boundingBox = builder.RegisterNestedObject ("boundingBox", offsetof (PlayerWithBoundingBox, boundingBox),
                                                         BoundingBox::Reflect ().mapping),
            .mapping = builder.End (),
        };
    }();

    return reflection;
}

const AllFieldTypesStructure::Reflection &AllFieldTypesStructure::Reflect ()
{
    static auto reflection = [] () -> Reflection
    {
        StandardLayout::MappingBuilder builder;
        builder.Begin ("AllFieldTypesStructure", sizeof (AllFieldTypesStructure));

        return {
            .int8 = builder.RegisterInt8 ("int8", offsetof (AllFieldTypesStructure, int8)),
            .int16 = builder.RegisterInt16 ("int16", offsetof (AllFieldTypesStructure, int16)),
            .int32 = builder.RegisterInt32 ("int32", offsetof (AllFieldTypesStructure, int32)),
            .int64 = builder.RegisterInt64 ("int64", offsetof (AllFieldTypesStructure, int64)),

            .uint8 = builder.RegisterUInt8 ("uint8", offsetof (AllFieldTypesStructure, uint8)),
            .uint16 = builder.RegisterUInt16 ("uint16", offsetof (AllFieldTypesStructure, uint16)),
            .uint32 = builder.RegisterUInt32 ("uint32", offsetof (AllFieldTypesStructure, uint32)),
            .uint64 = builder.RegisterUInt64 ("uint64", offsetof (AllFieldTypesStructure, uint64)),

            .floating = builder.RegisterFloat ("floating", offsetof (AllFieldTypesStructure, floating)),
            .doubleFloating =
                builder.RegisterDouble ("doubleFloating", offsetof (AllFieldTypesStructure, doubleFloating)),

            .block = builder.RegisterBlock ("block", offsetof (AllFieldTypesStructure, block),
                                            sizeof (AllFieldTypesStructure::block)),

            .string = builder.RegisterString ("string", offsetof (AllFieldTypesStructure, string),
                                              sizeof (AllFieldTypesStructure::string)),
            .mapping = builder.End (),
        };
    }();

    return reflection;
}
} // namespace Emergence::Query::Test
