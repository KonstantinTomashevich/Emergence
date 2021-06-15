#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>

using namespace Emergence::Pegasus::Test;

struct EntityIdLookupRequest
{
    decltype (Record::entityId) entityId;
};

struct NicknameLookupRequest
{
    decltype (Record::nickname) nickname;
};

struct NicknameAndEntityIdLookupRequest
{
    decltype (Record::nickname) nickname;
    decltype (Record::entityId) entityId;
};

constexpr Record ChangeEntityId (Record _record, uint32_t _newEntityId)
{
    _record.entityId = _newEntityId;
    return _record;
}

static Record firstRecord
    {
        0u,
        {
            "hugo"
        },
        {
            1u,
            0u,
        },
        {
            47u,
            96.0f,
        },
        {
            3.0f,
            7.0f,
            49.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_STUNNED,
    };

static Record anotherRecordForEntity0
    {
        0u,
        {
            "duplicate"
        },
        {
            2u,
            1u,
        },
        {
            49u,
            91.0f,
        },
        {
            5.0f,
            1.0f,
            0.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_IMMOBILIZED,
    };

static Record secondRecord
    {
        1u,
        {
            "karl"
        },
        {
            2u,
            1u,
        },
        {
            53u,
            111.0f,
        },
        {
            11.0f,
            12.0f,
            79.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_IMMOBILIZED,
    };

static Record secondRecordWithEntity0 = ChangeEntityId (secondRecord, 0u);

namespace Requests
{
EntityIdLookupRequest entity0 {0u};

EntityIdLookupRequest entity1 {1u};

EntityIdLookupRequest entity2 {2u};

NicknameLookupRequest hugo {{"hugo"}};

NicknameLookupRequest karl {{"karl"}};

NicknameAndEntityIdLookupRequest karlEntity1 {{"karl"}, 1u};

NicknameAndEntityIdLookupRequest hugoEntity1 {{"hugo"}, 1u};
};

BOOST_AUTO_TEST_SUITE (HashIndex)

BOOST_DATA_TEST_CASE(
    TestRoutine, boost::unit_test::data::monomorphic::collection (
    std::vector <Scenario>
        {
            {
                Record::Reflection::GetMapping (),
                {
                    CreateHashIndex {"source", {Record::Reflection::entityId}},
                    CheckIndexCanBeDropped {"source", true},
                    CopyIndexReference {"source", "duplicate"},
                    CheckIndexCanBeDropped {"source", false},
                    CheckIndexCanBeDropped {"duplicate", false},
                    RemoveIndexReference {"source"},
                    CheckIndexCanBeDropped {"duplicate", true},
                    DropIndex {"duplicate"},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateHashIndex {"entity", {Record::Reflection::entityId}},
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    AllocateAndInit {&secondRecord},
                    CloseAllocator {},
                    HashIndexLookupToRead {{"entity", "entity0", &Requests::entity0}},
                    CursorCheck {"entity0", &firstRecord},
                    CursorIncrement {"entity0"},
                    CursorCheck {"entity0", nullptr},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateHashIndex {"entity", {Record::Reflection::entityId}},
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    AllocateAndInit {&secondRecord},
                    CloseAllocator {},

                    CheckIndexCanBeDropped {"entity", true},
                    HashIndexLookupToRead {{"entity", "entity0", &Requests::entity0}},
                    CheckIndexCanBeDropped {"entity", false},

                    CursorCheck {"entity0", &firstRecord},
                    CopyCursor {"entity0", "entity0Copy"},

                    CursorIncrement {"entity0"},
                    CursorCheck {"entity0", nullptr},
                    CursorCheck {"entity0Copy", &firstRecord},

                    MoveCursor {"entity0Copy", "entity0Move"},
                    CursorCheck {"entity0Move", &firstRecord},
                    CheckIndexCanBeDropped {"entity", false},

                    CloseCursor {"entity0"},
                    CheckIndexCanBeDropped {"entity", false},
                    CloseCursor {"entity0Move"},
                    CheckIndexCanBeDropped {"entity", true},

                    HashIndexLookupToEdit {{"entity", "entity0", &Requests::entity0}},
                    CheckIndexCanBeDropped {"entity", false},
                    CursorCheck {"entity0", &firstRecord},
                    MoveCursor {"entity0", "entity0Move"},
                    CheckIndexCanBeDropped {"entity", false},

                    CursorCheck {"entity0Move", &firstRecord},
                    CloseCursor {"entity0Move"},
                    CheckIndexCanBeDropped {"entity", true},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateHashIndex {"entity", {Record::Reflection::entityId}},
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    AllocateAndInit {&secondRecord},
                    CloseAllocator {},
                    HashIndexLookupToRead {{"entity", "entity0", &Requests::entity0}},
                    CursorCheck {"entity0", &firstRecord},
                    HashIndexLookupToRead {{"entity", "entity1", &Requests::entity1}},
                    CursorCheck {"entity1", &secondRecord},
                    HashIndexLookupToRead {{"entity", "entity2", &Requests::entity2}},
                    CursorCheck {"entity2", nullptr},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    CloseAllocator {},
                    CreateHashIndex {"entity", {Record::Reflection::entityId}},
                    HashIndexLookupToRead {{"entity", "entity0", &Requests::entity0}},
                    CursorCheck {"entity0", &firstRecord},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateHashIndex {"entity", {Record::Reflection::entityId}},
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    AllocateAndInit {&secondRecord},
                    AllocateAndInit {&anotherRecordForEntity0},
                    CloseAllocator {},
                    HashIndexLookupToRead {{"entity", "entity0", &Requests::entity0}},
                    CursorCheckAllUnordered {"entity0", {&firstRecord, &anotherRecordForEntity0}},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateHashIndex {"entity", {Record::Reflection::entityId}},
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    AllocateAndInit {&secondRecord},
                    CloseAllocator {},
                    HashIndexLookupToEdit {{"entity", "entity1", &Requests::entity1}},
                    CursorCheck {"entity1", &secondRecord},
                    CursorEdit {"entity1", &secondRecordWithEntity0},
                    CloseCursor {"entity1"},
                    HashIndexLookupToRead {{"entity", "entity0", &Requests::entity0}},
                    CursorCheckAllUnordered {"entity0", {&firstRecord, &secondRecordWithEntity0}},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    AllocateAndInit {&secondRecord},
                    CloseAllocator {},
                    CreateHashIndex {"nickname", {Record::Reflection::nickname}},
                    HashIndexLookupToRead {{"nickname", "karl", &Requests::karl}},
                    CursorCheck {"karl", &secondRecord},
                    HashIndexLookupToRead {{"nickname", "hugo", &Requests::hugo}},
                    CursorCheck {"hugo", &firstRecord},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    AllocateAndInit {&secondRecord},
                    CloseAllocator {},
                    CreateHashIndex {"nicknameAndEntityId",
                                     {Record::Reflection::nickname, Record::Reflection::entityId}},
                    HashIndexLookupToRead {{"nicknameAndEntityId", "hugoEntity1", &Requests::hugoEntity1}},
                    CursorCheck {"hugoEntity1", nullptr},
                    CloseCursor {"hugoEntity1"},
                    HashIndexLookupToEdit {{"nicknameAndEntityId", "karlEntity1", &Requests::karlEntity1}},
                    CursorCheck {"karlEntity1", &secondRecord},
                    CursorEdit {"karlEntity1", &secondRecordWithEntity0},
                    CloseCursor {"karlEntity1"},
                    HashIndexLookupToRead {{"nicknameAndEntityId", "karlEntity1", &Requests::karlEntity1}},
                    CursorCheck {"karlEntity1", nullptr},
                }
            },
        }))
{
    sample.Execute ();
}

BOOST_AUTO_TEST_SUITE_END ()