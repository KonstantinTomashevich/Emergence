#include <cassert>

#include <Query/Test/Common.hpp>
#include <Query/Test/Data.hpp>
#include <Query/Test/RangeQueryTests.hpp>

namespace Emergence::Query::Test::RangeQuery
{
using namespace Tasks;

constexpr uint8_t FLAG_PLAYER_ID_SOURCE = 1u;

constexpr uint8_t FLAG_PLAYER_NAME_SOURCE = 1u << 1u;

static Storage RequestPlayerStorage (const std::vector <const void *> &_objects, uint8_t _sources)
{
    Storage storage
        {
            Player::Reflection::GetMapping (),
            _objects,
            {}
        };

    assert (_sources > 0u);
    if (_sources & FLAG_PLAYER_ID_SOURCE)
    {
        storage.sources.emplace_back (Sources::Range {"playerId", Player::Reflection::id});
    }

    if (_sources & FLAG_PLAYER_NAME_SOURCE)
    {
        storage.sources.emplace_back (Sources::Range {"playerName", Player::Reflection::name});
    }

    return storage;
}

static Storage RequestOrderingStorage (StandardLayout::FieldId _field)
{
    return
        {
            AllFieldTypesStructure::Reflection::GetMapping (),
            {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2},
            {
                Sources::Range
                    {
                        "ordering",
                        _field
                    },
            }
        };
}

Scenario CursorManipulations () noexcept
{
    return
        {
            {
                RequestPlayerStorage (
                    {&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                    FLAG_PLAYER_ID_SOURCE),
            },
            TestCursorCopyAndMove (
                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                QueryRangeToEdit {{{"playerId", "all"}, nullptr, nullptr}},
                &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &HUGO_0_ALIVE_STUNNED)
        };
}

Scenario ReversedCursorManipulations () noexcept
{
    return
        {
            {
                RequestPlayerStorage (
                    {&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                    FLAG_PLAYER_ID_SOURCE),
            },
            TestCursorCopyAndMove (
                QueryReversedRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                QueryReversedRangeToEdit {{{"playerId", "all"}, nullptr, nullptr}},
                &XAVIER_2_ALIVE_POISONED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED)
        };
}

Scenario SimpleLookups () noexcept
{
    return
        {
            {
                RequestPlayerStorage (
                    {&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                    FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

                QueryReversedRangeToRead {{{"playerId", "allReversed"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"allReversed",
                                       {&XAVIER_2_ALIVE_POISONED, &KARL_1_ALIVE_IMMOBILIZED, &HUGO_0_ALIVE_STUNNED}},

                QueryRangeToRead {{{"playerId", "ids[1,2]"}, &Queries::ID_1, &Queries::ID_2}},
                CursorCheckAllOrdered {"ids[1,2]", {&KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

                QueryRangeToRead {{{"playerId", "ids[0]"}, &Queries::ID_0, &Queries::ID_0}},
                CursorCheckAllOrdered {"ids[0]", {&HUGO_0_ALIVE_STUNNED}},

                QueryReversedRangeToRead {{{"playerId", "ids[1,2,3]Reversed"},
                                              &Queries::ID_0, &Queries::ID_2}},
                CursorCheckAllOrdered {"ids[1,2,3]Reversed",
                                       {&XAVIER_2_ALIVE_POISONED, &KARL_1_ALIVE_IMMOBILIZED, &HUGO_0_ALIVE_STUNNED}},

                QueryRangeToRead {{{"playerId", "ids[2,3]"}, &Queries::ID_2, &Queries::ID_3}},
                CursorCheckAllOrdered {"ids[2,3]", {&XAVIER_2_ALIVE_POISONED}},

                QueryRangeToRead {{{"playerId", "ids[3+]"}, &Queries::ID_3, nullptr}},
                CursorCheck {"ids[3+]", nullptr},

                QueryRangeToRead {{{"playerId", "ids[-0]"}, nullptr, &Queries::ID_0}},
                CursorCheckAllOrdered {"ids[-0]", {&HUGO_0_ALIVE_STUNNED}},
            }
        };
}

Scenario OnStringField () noexcept
{
    return
        {
            {
                RequestPlayerStorage (
                    {&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                    FLAG_PLAYER_NAME_SOURCE),
            },
            {
                QueryRangeToRead {{{"playerName", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

                QueryRangeToRead {{{"playerName", "ioran-zanek"}, &Queries::IORAN, &Queries::ZANEK}},
                CursorCheckAllOrdered {"ioran-zanek", {&KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

                QueryRangeToRead {{{"playerName", "hugo-karl"}, &Queries::HUGO, &Queries::KARL}},
                CursorCheckAllOrdered {"hugo-karl", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}},

                QueryRangeToRead {{{"playerName", "xavier"}, &Queries::XAVIER, &Queries::XAVIER}},
                CursorCheckAllOrdered {"xavier", {&XAVIER_2_ALIVE_POISONED}},

                QueryReversedRangeToRead {{{"playerName", "zanek-karl"}, &Queries::KARL, &Queries::ZANEK}},
                CursorCheckAllOrdered {"zanek-karl", {&XAVIER_2_ALIVE_POISONED, &KARL_1_ALIVE_IMMOBILIZED}},

                QueryReversedRangeToRead {{{"playerName", "null-xavier"}, nullptr, &Queries::XAVIER}},
                CursorCheckAllOrdered {"null-xavier",
                                       {&XAVIER_2_ALIVE_POISONED, &KARL_1_ALIVE_IMMOBILIZED, &HUGO_0_ALIVE_STUNNED}},
            }
        };
}

Scenario WithDuplicates () noexcept
{
    return
        {
            {
                RequestPlayerStorage (
                    {
                        &XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED,
                        &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED
                    },
                    FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {
                    "all", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                            &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},

                QueryRangeToRead {{{"playerId", "1"}, &Queries::ID_1, &Queries::ID_1}},
                CursorCheckAllOrdered {
                    "1", {&KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED}},

                QueryReversedRangeToRead {{{"playerId", "null-1(Reversed)"}, nullptr, &Queries::ID_1}},
                CursorCheckAllOrdered {"null-1(Reversed)",
                                       {&KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &HUGO_0_ALIVE_STUNNED}},
            }
        };
}

Scenario Edition () noexcept
{
    return
        {
            {
                RequestPlayerStorage (
                    {
                        &XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED,
                        &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED
                    },
                    FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"all"},

                QueryRangeToEdit {{{"playerId", "1-1"}, &Queries::ID_1, &Queries::ID_1}},
                CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1-1", &XAVIER_2_ALIVE_POISONED},
                CursorIncrement {"1-1"},
                CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1-1", &HUGO_0_ALIVE_STUNNED},
                CursorClose {"1-1"},

                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&HUGO_0_ALIVE_STUNNED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &XAVIER_2_ALIVE_POISONED, &XAVIER_2_ALIVE_POISONED}},
            }
        };
}

Scenario Deletion () noexcept
{
    return
        {
            {
                RequestPlayerStorage (
                    {&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                    FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"all"},

                QueryRangeToEdit {{{"playerId", "1-1"}, &Queries::ID_1, &Queries::ID_1}},
                CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorDeleteRecord {"1-1"},
                CursorCheck {"1-1", nullptr},
                CursorClose {"1-1"},

                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&HUGO_0_ALIVE_STUNNED, &XAVIER_2_ALIVE_POISONED}},
            }
        };
}

Scenario EditionAndDeletionFromReversedCursor () noexcept
{
    return
        {
            {
                RequestPlayerStorage (
                    {&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
                    FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"all"},

                QueryReversedRangeToEdit {{{"playerId", "null-1"}, nullptr, &Queries::ID_1}},
                CursorCheck {"null-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"null-1", &XAVIER_2_ALIVE_POISONED},
                CursorIncrement {"null-1"},
                CursorCheck {"null-1", &HUGO_0_ALIVE_STUNNED},
                CursorEdit {"null-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorClose {"null-1"},

                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all",
                                       {&KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED, &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"all"},

                QueryReversedRangeToEdit {{{"playerId", "2-2"}, &Queries::ID_2, &Queries::ID_2}},
                CursorCheck {"2-2", &XAVIER_2_ALIVE_POISONED},
                CursorDeleteRecord {"2-2"},
                CursorCheck {"2-2", &XAVIER_2_ALIVE_POISONED},
                CursorClose {"2-2"},

                QueryRangeToRead {{{"playerId", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"all"},
            }
        };
}

Scenario MultipleSourcesEditionAndDeletion () noexcept
{
    return
        {
            {
                RequestPlayerStorage (
                    {&XAVIER_2_ALIVE_POISONED, &HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED,
                     &KARL_1_ALIVE_IMMOBILIZED},
                    FLAG_PLAYER_ID_SOURCE | FLAG_PLAYER_NAME_SOURCE),
            },
            {
                QueryRangeToRead {{{"playerId", "allIds"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"allIds",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"allIds"},

                QueryRangeToRead {{{"playerName", "allNames"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"allNames",
                                       {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_1_ALIVE_IMMOBILIZED,
                                        &XAVIER_2_ALIVE_POISONED}},
                CursorClose {"allNames"},

                QueryRangeToEdit {{{"playerId", "1-1"}, &Queries::ID_1, &Queries::ID_1}},
                CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1-1", &XAVIER_2_ALIVE_POISONED},
                CursorDeleteRecord {"1-1"},
                CursorCheck {"1-1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1-1", &XAVIER_2_ALIVE_POISONED},
                CursorClose {"1-1"},

                QueryRangeToRead {{{"playerId", "allIds"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"allIds",
                                       {&HUGO_0_ALIVE_STUNNED, &XAVIER_2_ALIVE_POISONED, &XAVIER_2_ALIVE_POISONED}},

                QueryRangeToRead {{{"playerName", "allNames"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"allNames",
                                       {&HUGO_0_ALIVE_STUNNED, &XAVIER_2_ALIVE_POISONED, &XAVIER_2_ALIVE_POISONED}},
            }
        };
}

Scenario OrderingInt8 () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::int8),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}
            }
        };
}

Scenario OrderingInt16 () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::int16),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_0}}
            }
        };
}

Scenario OrderingInt32 () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::int32),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}
            }
        };
}

Scenario OrderingInt64 () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::int64),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2}}
            }
        };
}

Scenario OrderingUInt8 () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::uint8),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}
            }
        };
}

Scenario OrderingUInt16 () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::uint16),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}
            }
        };
}

Scenario OrderingUInt32 () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::uint32),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}
            }
        };
}

Scenario OrderingUInt64 () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::uint64),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2}}
            }
        };
}

Scenario OrderingFloat () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::floating),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1}}
            }
        };
}

Scenario OrderingDouble () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::doubleFloating),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2}}
            }
        };
}

Scenario OrderingBlock () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::block),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_2, &ALL_FIELD_TYPES_1}}
            }
        };
}

Scenario OrderingString () noexcept
{
    return
        {
            {
                RequestOrderingStorage (AllFieldTypesStructure::Reflection::string),
            },
            {
                QueryRangeToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", {&ALL_FIELD_TYPES_0, &ALL_FIELD_TYPES_1, &ALL_FIELD_TYPES_2}}
            }
        };
}
} // namespace Emergence::Query::Test::RangeQuery