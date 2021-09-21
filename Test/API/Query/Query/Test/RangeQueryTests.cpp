#include <cassert>

#include <Query/Test/Data.hpp>
#include <Query/Test/RangeQueryTests.hpp>

namespace Emergence::Query::Test::RangeQuery
{
using namespace Tasks;

constexpr uint8_t FLAG_PLAYER_ID_SOURCE = 1u;

constexpr uint8_t FLAG_PLAYER_NAME_SOURCE = 1u << 1u;

static Storage RequestPlayerStorage (const std::vector<const void *> &_objects, uint8_t _sources)
{
    Storage storage {Player::Reflect ().mapping, _objects, {}};

    assert (_sources > 0u);
    if (_sources & FLAG_PLAYER_ID_SOURCE)
    {
        storage.sources.emplace_back (Sources::Range {"playerId", Player::Reflect ().id});
    }

    if (_sources & FLAG_PLAYER_NAME_SOURCE)
    {
        storage.sources.emplace_back (Sources::Range {"playerName", Player::Reflect ().name});
    }

    return storage;
}

static Storage RequestOrderingStorage (const StandardLayout::FieldId &_field)
{
    return {AllFieldTypesStructure::Reflect ().mapping,
            {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2},
            {
                Sources::Range {"ordering", _field},
            }};
}

Scenario SimpleLookups ()
{
    return {
        {
            RequestPlayerStorage ({&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                                  FLAG_PLAYER_ID_SOURCE),
        },
        {
            QueryAscendingRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

            QueryDescendingRangeToRead {{{"playerId", "allDescending"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allDescending",
                                   {&XAVIER_2_ALIVE_POISONED, &KARL_1_ALIVE_IMMOBILIZED, &HUGO_0_ALIVE_STUNNED}},

            QueryAscendingRangeToRead {{{"playerId", "ids[1,2]"}, &Queries::ID_1, &Queries::ID_2}},
            CursorCheckAllOrdered {"ids[1,2]", {&KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

            QueryAscendingRangeToRead {{{"playerId", "ids[0]"}, &Queries::ID_0, &Queries::ID_0}},
            CursorCheckAllOrdered {"ids[0]", {&HUGO_0_ALIVE_STUNNED}},

            QueryDescendingRangeToRead {{{"playerId", "ids[1,2,3]Descending"}, &Queries::ID_0, &Queries::ID_2}},
            CursorCheckAllOrdered {"ids[1,2,3]Descending",
                                   {&XAVIER_2_ALIVE_POISONED, &KARL_1_ALIVE_IMMOBILIZED, &HUGO_0_ALIVE_STUNNED}},

            QueryAscendingRangeToRead {{{"playerId", "ids[2,3]"}, &Queries::ID_2, &Queries::ID_3}},
            CursorCheckAllOrdered {"ids[2,3]", {&XAVIER_2_ALIVE_POISONED}},

            QueryAscendingRangeToRead {{{"playerId", "ids[3+]"}, &Queries::ID_3, nullptr}},
            CursorCheck {"ids[3+]", nullptr},

            QueryAscendingRangeToRead {{{"playerId", "ids[-0]"}, nullptr, &Queries::ID_0}},
            CursorCheckAllOrdered {"ids[-0]", {&HUGO_0_ALIVE_STUNNED}},
        }};
}

Scenario OnStringField ()
{
    return {
        {
            RequestPlayerStorage ({&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                                  FLAG_PLAYER_NAME_SOURCE),
        },
        {
            QueryAscendingRangeToRead {{{"playerName", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

            QueryAscendingRangeToRead {{{"playerName", "ioran-zanek"}, &Queries::IORAN, &Queries::ZANEK}},
            CursorCheckAllOrdered {"ioran-zanek", {&KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

            QueryAscendingRangeToRead {{{"playerName", "hugo-karl"}, &Queries::HUGO, &Queries::KARL}},
            CursorCheckAllOrdered {"hugo-karl", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}},

            QueryAscendingRangeToRead {{{"playerName", "xavier"}, &Queries::XAVIER, &Queries::XAVIER}},
            CursorCheckAllOrdered {"xavier", {&XAVIER_2_ALIVE_POISONED}},

            QueryDescendingRangeToRead {{{"playerName", "zanek-karl"}, &Queries::KARL, &Queries::ZANEK}},
            CursorCheckAllOrdered {"zanek-karl", {&XAVIER_2_ALIVE_POISONED, &KARL_1_ALIVE_IMMOBILIZED}},

            QueryDescendingRangeToRead {{{"playerName", "null-xavier"}, nullptr, &Queries::XAVIER}},
            CursorCheckAllOrdered {"null-xavier",
                                   {&XAVIER_2_ALIVE_POISONED, &KARL_1_ALIVE_IMMOBILIZED, &HUGO_0_ALIVE_STUNNED}},
        }};
}

Scenario WithDuplicates ()
{
    return {{
                RequestPlayerStorage ({&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED,
                                       &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED},
                                      FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryAscendingRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

                QueryAscendingRangeToRead {{{"playerId", "1"}, &Queries::ID_1, &Queries::ID_1}},
                CursorCheckAllOrdered {
                    "1", {&KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED}},

                QueryDescendingRangeToRead {{{"playerId", "null-1(Descending)"}, nullptr, &Queries::ID_1}},
                CursorCheckAllOrdered {"null-1(Descending)",
                                       {&KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &HUGO_0_ALIVE_STUNNED}},
            }};
}

Scenario Edition ()
{
    return {{
                RequestPlayerStorage ({&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED,
                                       &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED},
                                      FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryAscendingRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"all"},

                QueryAscendingRangeToEdit {{{"playerId", "1-1"}, &Queries::ID_1, &Queries::ID_1}},
                CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1-1", &XAVIER_2_ALIVE_POISONED},
                CursorIncrement {"1-1"},
                CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1-1", &HUGO_0_ALIVE_STUNNED},
                CursorClose {"1-1"},

                QueryAscendingRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&HUGO_0_ALIVE_STUNNED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &XAVIER_2_ALIVE_POISONED, &XAVIER_2_ALIVE_POISONED}},
            }};
}

Scenario Deletion ()
{
    return {
        {
            RequestPlayerStorage ({&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                                  FLAG_PLAYER_ID_SOURCE),
        },
        {
            QueryAscendingRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},
            CursorClose {"all"},

            QueryAscendingRangeToEdit {{{"playerId", "1-1"}, &Queries::ID_1, &Queries::ID_1}},
            CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
            CursorDeleteObject {"1-1"},
            CursorCheck {"1-1", nullptr},
            CursorClose {"1-1"},

            QueryAscendingRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all", {&HUGO_0_ALIVE_STUNNED, &XAVIER_2_ALIVE_POISONED}},
        }};
}

Scenario EditionAndDeletionFromDescendingCursor ()
{
    return {
        {
            RequestPlayerStorage ({&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                                  FLAG_PLAYER_ID_SOURCE),
        },
        {
            QueryAscendingRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},
            CursorClose {"all"},

            QueryDescendingRangeToEdit {{{"playerId", "null-1"}, nullptr, &Queries::ID_1}},
            CursorCheck {"null-1", &KARL_1_ALIVE_IMMOBILIZED},
            CursorEdit {"null-1", &XAVIER_2_ALIVE_POISONED},
            CursorIncrement {"null-1"},
            CursorCheck {"null-1", &HUGO_0_ALIVE_STUNNED},
            CursorEdit {"null-1", &KARL_1_ALIVE_IMMOBILIZED},
            CursorClose {"null-1"},

            QueryAscendingRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all",
                                   {&KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED, &XAVIER_2_ALIVE_POISONED}},
            CursorClose {"all"},

            QueryDescendingRangeToEdit {{{"playerId", "2-2"}, &Queries::ID_2, &Queries::ID_2}},
            CursorCheck {"2-2", &XAVIER_2_ALIVE_POISONED},
            CursorDeleteObject {"2-2"},
            CursorCheck {"2-2", &XAVIER_2_ALIVE_POISONED},
            CursorClose {"2-2"},

            QueryAscendingRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all", {&KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},
            CursorClose {"all"},
        }};
}

Scenario MultipleSourcesEditionAndDeletion ()
{
    return {{
                RequestPlayerStorage ({&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED,
                                       &KARL_1_ALIVE_IMMOBILIZED},
                                      FLAG_PLAYER_ID_SOURCE | FLAG_PLAYER_NAME_SOURCE),
            },
            {
                QueryAscendingRangeToRead {{{"playerId", "allIds"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"allIds",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"allIds"},

                QueryAscendingRangeToRead {{{"playerName", "allNames"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"allNames",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"allNames"},

                QueryAscendingRangeToEdit {{{"playerId", "1-1"}, &Queries::ID_1, &Queries::ID_1}},
                CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1-1", &XAVIER_2_ALIVE_POISONED},
                CursorDeleteObject {"1-1"},
                CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1-1", &XAVIER_2_ALIVE_POISONED},
                CursorClose {"1-1"},

                QueryAscendingRangeToRead {{{"playerId", "allIds"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"allIds",
                                       {&HUGO_0_ALIVE_STUNNED, &XAVIER_2_ALIVE_POISONED, &XAVIER_2_ALIVE_POISONED}},

                QueryAscendingRangeToRead {{{"playerName", "allNames"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"allNames",
                                       {&HUGO_0_ALIVE_STUNNED, &XAVIER_2_ALIVE_POISONED, &XAVIER_2_ALIVE_POISONED}},
            }};
}

Scenario OrderingInt8 ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().int8),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}}};
}

Scenario OrderingInt16 ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().int16),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_0}}}};
}

Scenario OrderingInt32 ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().int32),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}}};
}

Scenario OrderingInt64 ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().int64),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2}}}};
}

Scenario OrderingUInt8 ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().uint8),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}}};
}

Scenario OrderingUInt16 ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().uint16),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}}};
}

Scenario OrderingUInt32 ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().uint32),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}}};
}

Scenario OrderingUInt64 ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().uint64),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2}}}};
}

Scenario OrderingFloat ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().floating),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1}}}};
}

Scenario OrderingDouble ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().doubleFloating),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2}}}};
}

Scenario OrderingBlock ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().block),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}}};
}

Scenario OrderingString ()
{
    return {{
                RequestOrderingStorage (AllFieldTypesStructure::Reflect ().string),
            },
            {QueryAscendingRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
             CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2}}}};
}
} // namespace Emergence::Query::Test::RangeQuery
