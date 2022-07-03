#include <Query/Test/DataTypes.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Query::Test
{
const Player::Reflection &Player::Reflect ()
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Player);

        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_STRING (name);
        EMERGENCE_MAPPING_REGISTER_REGULAR (classId);

        EMERGENCE_MAPPING_REGISTER_BIT (alive, status, Status::FLAG_ALIVE_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (stunned, status, Status::FLAG_STUNNED_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (poisoned, status, Status::FLAG_POISONED_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (immobilized, status, Status::FLAG_IMMOBILIZED_OFFSET);

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const BoundingBox::Reflection &BoundingBox::Reflect ()
{
    static Reflection reflection = [] () -> Reflection
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (BoundingBox);

        EMERGENCE_MAPPING_REGISTER_REGULAR (minX);
        EMERGENCE_MAPPING_REGISTER_REGULAR (minY);
        EMERGENCE_MAPPING_REGISTER_REGULAR (minZ);

        EMERGENCE_MAPPING_REGISTER_REGULAR (maxX);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maxY);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maxZ);

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const ScreenRect::Reflection &ScreenRect::Reflect ()
{
    static Reflection reflection = [] () -> Reflection
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ScreenRect);

        EMERGENCE_MAPPING_REGISTER_REGULAR (minX);
        EMERGENCE_MAPPING_REGISTER_REGULAR (minY);

        EMERGENCE_MAPPING_REGISTER_REGULAR (maxX);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maxY);

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const PlayerWithBoundingBox::Reflection &PlayerWithBoundingBox::Reflect ()
{
    static Reflection reflection = [] () -> Reflection
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PlayerWithBoundingBox);

        EMERGENCE_MAPPING_REGISTER_REGULAR (player);
        EMERGENCE_MAPPING_REGISTER_REGULAR (boundingBox);

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const AllFieldTypesStructure::Reflection &AllFieldTypesStructure::Reflect ()
{
    static Reflection reflection = [] () -> Reflection
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AllFieldTypesStructure);

        EMERGENCE_MAPPING_REGISTER_REGULAR (int8);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int16);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int64);

        EMERGENCE_MAPPING_REGISTER_REGULAR (uint8);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint16);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint64);

        EMERGENCE_MAPPING_REGISTER_REGULAR (floating);
        EMERGENCE_MAPPING_REGISTER_REGULAR (doubleFloating);

        EMERGENCE_MAPPING_REGISTER_STRING (string);
        EMERGENCE_MAPPING_REGISTER_BLOCK (block);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniqueString);

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Query::Test
