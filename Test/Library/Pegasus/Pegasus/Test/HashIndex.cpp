#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>
#include <Pegasus/Test/Shortcuts.hpp>

#include <Testing/Testing.hpp>

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

struct AliveLookupRequest
{
    uint8_t aliveFlag;
};

struct AliveAndStunnedLookupRequest
{
    uint8_t aliveFlag;
    uint8_t stunnedFlag;
};

constexpr Record ChangeEntityId (Record _record, uint32_t _newEntityId) noexcept
{
    _record.entityId = _newEntityId;
    return _record;
}

static const Record firstRecord
    {
        0u,
        {
            "hugo"
        },
        {
            3.0f,
            7.0f,
            0.0f,
            4.0f,
            8.0f,
            0.0f,
        },
        {
            0u,
            0u,
            0u,
            0u,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_STUNNED,
    };

static const Record anotherRecordForEntity0
    {
        0u,
        {
            "duplicate"
        },
        {
            5.0f,
            1.0f,
            0.0f,
            6.0f,
            2.0f,
            0.0f,
        },
        {
            0u,
            0u,
            0u,
            0u,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_IMMOBILIZED,
    };

static const Record secondRecord
    {
        1u,
        {
            "karl"
        },
        {
            11.0f,
            12.0f,
            0.0f,
            12.0f,
            13.0f,
            0.0f,
        },
        {
            0u,
            0u,
            0u,
            0u,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_IMMOBILIZED,
    };

static const Record secondRecordWithEntity0 = ChangeEntityId (secondRecord, 0u);

namespace Requests
{
static const EntityIdLookupRequest entity0 {0u};

static const EntityIdLookupRequest entity1 {1u};

static const EntityIdLookupRequest entity2 {2u};

static const NicknameLookupRequest hugo {{"hugo"}};

static const NicknameLookupRequest karl {{"karl"}};

static const NicknameAndEntityIdLookupRequest karlEntity1 {{"karl"}, 1u};

static const NicknameAndEntityIdLookupRequest karlEntity0 {{"karl"}, 0u};

static const NicknameAndEntityIdLookupRequest hugoEntity1 {{"hugo"}, 1u};

static const AliveLookupRequest alive {Record::Status::FLAG_ALIVE};

static const AliveLookupRequest dead {0u};

static const AliveAndStunnedLookupRequest aliveAndStunned {Record::Status::FLAG_ALIVE, Record::Status::FLAG_STUNNED};

static const AliveAndStunnedLookupRequest aliveAndNotStunned {Record::Status::FLAG_ALIVE, 0u};
};

BEGIN_SUITE (HashIndex)

TEST_CASE (ReferenceManipulations)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            std::vector <Task>
                {
                    CreateHashIndex {"source", {Record::Reflection::entityId}},
                } +
            Shortcuts::TestIsCanBeDropped ("source")
        };
}

TEST_CASE (SimpleLookup)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                CreateHashIndex {"entity", {Record::Reflection::entityId}},
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                CloseAllocator {},
                HashIndexLookupToRead {{{"entity", "entity0"}, &Requests::entity0}},
                CursorCheck {"entity0", &firstRecord},
                CursorIncrement {"entity0"},
                CursorCheck {"entity0", nullptr},
            }
        };
}

TEST_CASE (CursorManipulations)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            std::vector <Task>
                {
                    CreateHashIndex {"entity", {Record::Reflection::entityId}},
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    AllocateAndInit {&secondRecord},
                    CloseAllocator {},
                } +
            Shortcuts::TestCursorCopyAndMove (
                HashIndexLookupToRead {{{"entity", "entity0"}, &Requests::entity0}},
                HashIndexLookupToEdit {{{"entity", "entity0"}, &Requests::entity0}},
                &firstRecord, nullptr, &firstRecord)
        };
}

TEST_CASE (LookupForNonExistentRecord)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                CreateHashIndex {"entity", {Record::Reflection::entityId}},
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                CloseAllocator {},
                HashIndexLookupToRead {{{"entity", "entity0"}, &Requests::entity0}},
                CursorCheckAllUnordered {"entity0", {&firstRecord}},
                HashIndexLookupToRead {{{"entity", "entity1"}, &Requests::entity1}},
                CursorCheckAllUnordered {"entity1", {&secondRecord}},
                HashIndexLookupToRead {{{"entity", "entity2"}, &Requests::entity2}},
                CursorCheck {"entity2", nullptr},
            }
        };
}

TEST_CASE (InsertBeforeCreation)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                CloseAllocator {},
                CreateHashIndex {"entity", {Record::Reflection::entityId}},
                HashIndexLookupToRead {{{"entity", "entity0"}, &Requests::entity0}},
                CursorCheckAllUnordered {"entity0", {&firstRecord}},
            }
        };
}

TEST_CASE (LookupForMany)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                CreateHashIndex {"entity", {Record::Reflection::entityId}},
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                AllocateAndInit {&anotherRecordForEntity0},
                CloseAllocator {},
                HashIndexLookupToRead {{{"entity", "entity0"}, &Requests::entity0}},
                CursorCheckAllUnordered {"entity0", {&firstRecord, &anotherRecordForEntity0}},
            }
        };
}

TEST_CASE (LookupAndEdit)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                CreateHashIndex {"entity", {Record::Reflection::entityId}},
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                CloseAllocator {},
                HashIndexLookupToEdit {{{"entity", "entity1"}, &Requests::entity1}},
                CursorCheck {"entity1", &secondRecord},
                CursorEdit {"entity1", &secondRecordWithEntity0},
                CloseCursor {"entity1"},
                HashIndexLookupToRead {{{"entity", "entity0"}, &Requests::entity0}},
                CursorCheckAllUnordered {"entity0", {&firstRecord, &secondRecordWithEntity0}},
            }
        };
}

TEST_CASE (OnStringField)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                CloseAllocator {},
                CreateHashIndex {"nickname", {Record::Reflection::nickname}},
                HashIndexLookupToRead {{{"nickname", "karl"}, &Requests::karl}},
                CursorCheckAllUnordered {"karl", {&secondRecord}},
                HashIndexLookupToRead {{{"nickname", "hugo"}, &Requests::hugo}},
                CursorCheckAllUnordered {"hugo", {&firstRecord}},
            }
        };
}

TEST_CASE (OnTwoFields)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                CloseAllocator {},
                CreateHashIndex {"nicknameAndEntityId",
                                 {Record::Reflection::nickname, Record::Reflection::entityId}},
                HashIndexLookupToRead {{{"nicknameAndEntityId", "hugoEntity1"}, &Requests::hugoEntity1}},
                CursorCheck {"hugoEntity1", nullptr},
                HashIndexLookupToRead {{{"nicknameAndEntityId", "karlEntity1"}, &Requests::karlEntity1}},
                CursorCheckAllUnordered {"karlEntity1", {&secondRecord}},
            }
        };
}

TEST_CASE (OnBitField)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                CloseAllocator {},
                CreateHashIndex {"alive", {Record::Reflection::alive}},
                HashIndexLookupToRead {{{"alive", "alive"}, &Requests::alive}},
                CursorCheckAllUnordered {"alive", {&firstRecord, &secondRecord}},
                HashIndexLookupToRead {{{"alive", "dead"}, &Requests::dead}},
                CursorCheck {"dead", nullptr},
            }
        };
}

TEST_CASE (OnTwoBitFields)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                CloseAllocator {},
                CreateHashIndex {"aliveAndStunned", {Record::Reflection::alive, Record::Reflection::stunned}},
                HashIndexLookupToRead {{{"aliveAndStunned", "aliveAndStunned"}, &Requests::aliveAndStunned}},
                CursorCheckAllUnordered {"aliveAndStunned", {&firstRecord}},
                HashIndexLookupToRead {{{"aliveAndStunned", "aliveAndNotStunned"}, &Requests::aliveAndNotStunned}},
                CursorCheckAllUnordered {"aliveAndNotStunned", {&secondRecord}},
            }
        };
}

TEST_CASE (MultipleIndicesEdition)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                CloseAllocator {},
                CreateHashIndex {"entityId", {Record::Reflection::entityId}},
                CreateHashIndex {"nickname", {Record::Reflection::nickname}},
                CreateHashIndex {"nicknameAndEntityId",
                                 {Record::Reflection::nickname, Record::Reflection::entityId}},

                HashIndexLookupToEdit {{{"entityId", "entity1"}, &Requests::entity1}},
                CursorCheck {"entity1", &secondRecord},
                CursorEdit {"entity1", &secondRecordWithEntity0},
                CloseCursor {"entity1"},

                HashIndexLookupToRead {{{"entityId", "entity0"}, &Requests::entity0}},
                CursorCheckAllUnordered {"entity0", {&firstRecord, &secondRecordWithEntity0}},

                HashIndexLookupToRead {{{"entityId", "entity1"}, &Requests::entity1}},
                CursorCheck {"entity1", nullptr},

                HashIndexLookupToRead {{{"nickname", "karl"}, &Requests::karl}},
                CursorCheckAllUnordered {"karl", {&secondRecordWithEntity0}},

                HashIndexLookupToRead {{{"nickname", "hugo"}, &Requests::hugo}},
                CursorCheckAllUnordered {"hugo", {&firstRecord}},

                HashIndexLookupToRead {{{"nicknameAndEntityId", "karlEntity1"}, &Requests::karlEntity1}},
                CursorCheck {"karlEntity1", nullptr},

                HashIndexLookupToRead {{{"nicknameAndEntityId", "karlEntity0"}, &Requests::karlEntity0}},
                CursorCheckAllUnordered {"karlEntity0", {&secondRecordWithEntity0}},
            }
        };
}

TEST_CASE (MultipleIndicesDeletion)
{
    Scenario
        {
            Record::Reflection::GetMapping (),
            {
                CreateHashIndex {"entity", {Record::Reflection::entityId}},
                CreateHashIndex {"nickname", {Record::Reflection::nickname}},
                OpenAllocator {},
                AllocateAndInit {&firstRecord},
                AllocateAndInit {&secondRecord},
                CloseAllocator {},

                HashIndexLookupToEdit {{{"entity", "entity1"}, &Requests::entity1}},
                CursorCheck {"entity1", &secondRecord},
                CursorEdit {"entity1", &secondRecordWithEntity0},
                CursorDeleteRecord {"entity1"},
                CloseCursor {"entity1"},

                HashIndexLookupToRead {{{"entity", "entity0"}, &Requests::entity0}},
                CursorCheckAllUnordered {"entity0", {&firstRecord}},

                HashIndexLookupToRead {{{"entity", "entity1"}, &Requests::entity1}},
                CursorCheck {"entity1", nullptr},

                HashIndexLookupToRead {{{"nickname", "karl"}, &Requests::karl}},
                CursorCheck {"karl", nullptr},

                HashIndexLookupToRead {{{"nickname", "hugo"}, &Requests::hugo}},
                CursorCheckAllUnordered {"hugo", {&firstRecord}},
            }
        };
}

END_SUITE