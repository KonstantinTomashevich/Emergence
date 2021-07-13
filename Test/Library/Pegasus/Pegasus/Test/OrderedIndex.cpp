#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>
#include <Pegasus/Test/Shortcuts.hpp>

#include <StandardLayout/MappingBuilder.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Pegasus::Test;

/// Structure with all supported field types, used to check OrderedIndex comparators.
struct AllFieldTypesStructure
{
    struct Reflection final
    {
        Reflection () = delete;

        static Emergence::StandardLayout::Mapping GetMapping ();

        static Emergence::StandardLayout::FieldId int8;
        static Emergence::StandardLayout::FieldId int16;
        static Emergence::StandardLayout::FieldId int32;
        static Emergence::StandardLayout::FieldId int64;

        static Emergence::StandardLayout::FieldId uint8;
        static Emergence::StandardLayout::FieldId uint16;
        static Emergence::StandardLayout::FieldId uint32;
        static Emergence::StandardLayout::FieldId uint64;

        static Emergence::StandardLayout::FieldId floating;
        static Emergence::StandardLayout::FieldId doubleFloating;

        static Emergence::StandardLayout::FieldId block;
        static Emergence::StandardLayout::FieldId string;
    };

    int8_t int8 = 0;
    int16_t int16 = 0;
    int32_t int32 = 0;
    int64_t int64 = 0;

    uint8_t uint8 = 0u;
    uint16_t uint16 = 0u;
    uint32_t uint32 = 0u;
    uint64_t uint64 = 0u;

    float floating = 0.0f;
    double doubleFloating = 0.0;

    std::array <uint8_t, 4> block {};
    std::array <char, 24> string {};
};

static Emergence::StandardLayout::Mapping RegisterAllFieldTypesStructure ()
{
    Emergence::StandardLayout::MappingBuilder builder;
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

Emergence::StandardLayout::Mapping AllFieldTypesStructure::Reflection::GetMapping ()
{
    static Emergence::StandardLayout::Mapping mapping = RegisterAllFieldTypesStructure ();
    return mapping;
}

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::int8;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::int16;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::int32;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::int64;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::uint8;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::uint16;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::uint32;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::uint64;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::floating;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::doubleFloating;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::block;

Emergence::StandardLayout::FieldId AllFieldTypesStructure::Reflection::string;

static const AllFieldTypesStructure firstRecordWithAllFieldTypes
    {
        -3,
        258,
        -53400,
        6,

        1u,
        13u,
        79u,
        1111u,

        89.0f,
        12.3458,

        {13u, 12u, 15u, 21u},
        {"hugo"},
    };

static const AllFieldTypesStructure secondRecordWithAllFieldTypes
    {
        1,
        -233,
        170,
        182634,

        13u,
        1563u,
        173656u,
        1234u,

        111.0f,
        6.2356,

        {67u, 12u, 15u, 21u},
        {"karl"},
    };

static const AllFieldTypesStructure thirdRecordWithAllFieldTypes
    {
        0,
        2,
        -5,
        63674896,

        4u,
        37u,
        12341u,
        17939471u,

        45.0f,
        17.3458,

        {13u, 15u, 15u, 21u},
        {"xavier"},
    };

Scenario ExecuteOrderingTest (
    Emergence::StandardLayout::FieldId _field,
    const std::vector <const void *> &_expectedOrder)
{
    return
        {
            AllFieldTypesStructure::Reflection::GetMapping (),
            {
                CreateOrderedIndex {"ordering", _field},
                OpenAllocator {},
                AllocateAndInit {&firstRecordWithAllFieldTypes},
                AllocateAndInit {&secondRecordWithAllFieldTypes},
                AllocateAndInit {&thirdRecordWithAllFieldTypes},
                CloseAllocator {},

                OrderedIndexLookupToRead {{{"ordering", "all"}, nullptr, nullptr}},
                CursorCheckAllOrdered {"all", _expectedOrder},
            }
        };
}

struct EntityIdBound
{
    decltype (Record::entityId) entityId;
};

struct NicknameBound
{
    decltype (Record::nickname) nickname;
};

static const Record entity0Hugo
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

static const Record entity1Karl
    {
        1u,
        {
            "karl"
        },
        {
            5.0f,
            4.0f,
            0.0f,
            6.0f,
            5.0f,
            0.0f,
        },
        {
            0u,
            0u,
            0u,
            0u,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_POISONED,
    };

static const Record entity2Xavier
    {
        2u,
        {
            "xavier"
        },
        {
            9.0f,
            7.0f,
            0.0f,
            10.0f,
            8.0f,
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

namespace Bounds
{
static const EntityIdBound entity0 {0u};

static const EntityIdBound entity1 {1u};

static const EntityIdBound entity2 {2u};

static const EntityIdBound entity3 {3u};

static const NicknameBound hugo {{"hugo"}};

static const NicknameBound ioran {{"ioran"}};

static const NicknameBound karl {{"karl"}};

static const NicknameBound xavier {{"xavier"}};

static const NicknameBound zanek {{"zanek"}};
};

BEGIN_SUITE (OrderedIndex)

TEST_CASE (ReferenceManipulations)
{
    Scenario {
        Record::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateOrderedIndex {"source", Record::Reflection::entityId},
            } +
        Shortcuts::TestIsCanBeDropped ("source")
    };
}

TEST_CASE (CursorManipulations)
{
    Scenario {
        Record::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateOrderedIndex {"entity", Record::Reflection::entityId},
                OpenAllocator {},
                AllocateAndInit {&entity2Xavier},
                AllocateAndInit {&entity0Hugo},
                AllocateAndInit {&entity1Karl},
                CloseAllocator {},
            } +
        Shortcuts::TestCursorCopyAndMove (
            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            OrderedIndexLookupToEdit {{{"entity", "allEntities"}, nullptr, nullptr}},
            &entity0Hugo, &entity1Karl, &entity0Hugo)
    };
}

TEST_CASE (ReversedCursorManipulations)
{
    Scenario {
        Record::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateOrderedIndex {"entity", Record::Reflection::entityId},
                OpenAllocator {},
                AllocateAndInit {&entity2Xavier},
                AllocateAndInit {&entity0Hugo},
                AllocateAndInit {&entity1Karl},
                CloseAllocator {},
            } +
        Shortcuts::TestCursorCopyAndMove (
            OrderedIndexLookupToReadReversed {{{"entity", "allEntities"}, nullptr, nullptr}},
            OrderedIndexLookupToEditReversed {{{"entity", "allEntities"}, nullptr, nullptr}},
            &entity2Xavier, &entity1Karl, &entity2Xavier)
    };
}

TEST_CASE (SimpleLookups)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateOrderedIndex {"entity", Record::Reflection::entityId},
            OpenAllocator {},
            AllocateAndInit {&entity2Xavier},
            AllocateAndInit {&entity0Hugo},
            AllocateAndInit {&entity1Karl},
            CloseAllocator {},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity2Xavier}},

            OrderedIndexLookupToReadReversed {{{"entity", "allEntitiesReversed"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntitiesReversed", {&entity2Xavier, &entity1Karl, &entity0Hugo}},

            OrderedIndexLookupToRead {{{"entity", "entities[1,2]"}, &Bounds::entity1, &Bounds::entity2}},
            CursorCheckAllOrdered {"entities[1,2]", {&entity1Karl, &entity2Xavier}},

            OrderedIndexLookupToRead {{{"entity", "entities[0]"}, &Bounds::entity0, &Bounds::entity0}},
            CursorCheckAllOrdered {"entities[0]", {&entity0Hugo}},

            OrderedIndexLookupToReadReversed {{{"entity", "entities[1,2,3]Reversed"},
                                                  &Bounds::entity0, &Bounds::entity2}},
            CursorCheckAllOrdered {"entities[1,2,3]Reversed", {&entity2Xavier, &entity1Karl, &entity0Hugo}},

            OrderedIndexLookupToRead {{{"entity", "entities[2,3]"}, &Bounds::entity2, &Bounds::entity3}},
            CursorCheckAllOrdered {"entities[2,3]", {&entity2Xavier}},

            OrderedIndexLookupToRead {{{"entity", "entities[3+]"}, &Bounds::entity3, nullptr}},
            CursorCheck {"entities[3+]", nullptr},

            OrderedIndexLookupToRead {{{"entity", "entities[-0]"}, nullptr, &Bounds::entity0}},
            CursorCheckAllOrdered {"entities[-0]", {&entity0Hugo}},
        }
    };
}

TEST_CASE (InsertBeforeCreation)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            OpenAllocator {},
            AllocateAndInit {&entity2Xavier},
            AllocateAndInit {&entity0Hugo},
            AllocateAndInit {&entity1Karl},
            CloseAllocator {},
            CreateOrderedIndex {"entity", Record::Reflection::entityId},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity2Xavier}},
        }
    };
}

TEST_CASE (OnStringField)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            CreateOrderedIndex {"nickname", Record::Reflection::nickname},
            OpenAllocator {},
            AllocateAndInit {&entity2Xavier},
            AllocateAndInit {&entity0Hugo},
            AllocateAndInit {&entity1Karl},
            CloseAllocator {},

            OrderedIndexLookupToRead {{{"nickname", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all", {&entity0Hugo, &entity1Karl, &entity2Xavier}},

            OrderedIndexLookupToRead {{{"nickname", "ioran-zanek"}, &Bounds::ioran, &Bounds::zanek}},
            CursorCheckAllOrdered {"ioran-zanek", {&entity1Karl, &entity2Xavier}},

            OrderedIndexLookupToRead {{{"nickname", "hugo-karl"}, &Bounds::hugo, &Bounds::karl}},
            CursorCheckAllOrdered {"hugo-karl", {&entity0Hugo, &entity1Karl}},

            OrderedIndexLookupToRead {{{"nickname", "xavier"}, &Bounds::xavier, &Bounds::xavier}},
            CursorCheckAllOrdered {"xavier", {&entity2Xavier}},

            OrderedIndexLookupToReadReversed {{{"nickname", "zanek-karl"}, &Bounds::karl, &Bounds::zanek}},
            CursorCheckAllOrdered {"zanek-karl", {&entity2Xavier, &entity1Karl}},

            OrderedIndexLookupToReadReversed {{{"nickname", "xavier-null"}, nullptr, &Bounds::xavier}},
            CursorCheckAllOrdered {"xavier-null", {&entity2Xavier, &entity1Karl, &entity0Hugo}},
        }
    };
}

TEST_CASE (WithDuplicates)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            OpenAllocator {},
            AllocateAndInit {&entity2Xavier},
            AllocateAndInit {&entity0Hugo},
            AllocateAndInit {&entity1Karl},
            AllocateAndInit {&entity1Karl},
            AllocateAndInit {&entity1Karl},
            CloseAllocator {},
            CreateOrderedIndex {"entity", Record::Reflection::entityId},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities",
                                   {&entity0Hugo, &entity1Karl, &entity1Karl, &entity1Karl, &entity2Xavier}},

            OrderedIndexLookupToRead {{{"entity", "entity1"}, &Bounds::entity1, &Bounds::entity1}},
            CursorCheckAllOrdered {"entity1", {&entity1Karl, &entity1Karl, &entity1Karl}},

            OrderedIndexLookupToReadReversed {{{"entity", "entity[-1]Reversed"}, nullptr, &Bounds::entity1}},
            CursorCheckAllOrdered {"entity[-1]Reversed",
                                   {&entity1Karl, &entity1Karl, &entity1Karl, &entity0Hugo}},
        }
    };
}

TEST_CASE (Edition)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            OpenAllocator {},
            AllocateAndInit {&entity2Xavier},
            AllocateAndInit {&entity0Hugo},
            AllocateAndInit {&entity1Karl},
            AllocateAndInit {&entity1Karl},
            AllocateAndInit {&entity1Karl},
            CloseAllocator {},
            CreateOrderedIndex {"entity", Record::Reflection::entityId},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities",
                                   {&entity0Hugo, &entity1Karl, &entity1Karl, &entity1Karl, &entity2Xavier}},
            CloseCursor {"allEntities"},

            OrderedIndexLookupToEdit {{{"entity", "entity1"}, &Bounds::entity1, &Bounds::entity1}},
            CursorCheck {"entity1", &entity1Karl},
            CursorEdit {"entity1", &entity2Xavier},
            CursorIncrement {"entity1"},
            CursorCheck {"entity1", &entity1Karl},
            CursorEdit {"entity1", &entity0Hugo},
            CloseCursor {"entity1"},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities",
                                   {&entity0Hugo, &entity0Hugo, &entity1Karl, &entity2Xavier, &entity2Xavier}},
        }
    };
}

TEST_CASE (Deletion)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            OpenAllocator {},
            AllocateAndInit {&entity2Xavier},
            AllocateAndInit {&entity0Hugo},
            AllocateAndInit {&entity1Karl},
            CloseAllocator {},
            CreateOrderedIndex {"entity", Record::Reflection::entityId},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity2Xavier}},
            CloseCursor {"allEntities"},

            OrderedIndexLookupToEdit {{{"entity", "entity1"}, &Bounds::entity1, &Bounds::entity1}},
            CursorCheck {"entity1", &entity1Karl},
            CursorDeleteRecord {"entity1"},
            CursorCheck {"entity1", nullptr},
            CloseCursor {"entity1"},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity2Xavier}},
        }
    };
}

TEST_CASE (EditionAndDeletionFromReversedCursor)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            OpenAllocator {},
            AllocateAndInit {&entity2Xavier},
            AllocateAndInit {&entity0Hugo},
            AllocateAndInit {&entity1Karl},
            CloseAllocator {},
            CreateOrderedIndex {"entity", Record::Reflection::entityId},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity2Xavier}},
            CloseCursor {"allEntities"},

            OrderedIndexLookupToEditReversed {{{"entity", "entity[-1]"}, nullptr, &Bounds::entity1}},
            CursorCheck {"entity[-1]", &entity1Karl},
            CursorEdit {"entity[-1]", &entity2Xavier},
            CursorIncrement {"entity[-1]"},
            CursorCheck {"entity[-1]", &entity0Hugo},
            CursorEdit {"entity[-1]", &entity1Karl},
            CloseCursor {"entity[-1]"},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities", {&entity1Karl, &entity2Xavier, &entity2Xavier}},
            CloseCursor {"allEntities"},

            OrderedIndexLookupToEditReversed {{{"entity", "entity2"}, &Bounds::entity2, &Bounds::entity2}},
            CursorCheck {"entity2", &entity2Xavier},
            CursorDeleteRecord {"entity2"},
            CursorCheck {"entity2", &entity2Xavier},
            CloseCursor {"entity2"},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities", {&entity1Karl, &entity2Xavier}},
            CloseCursor {"allEntities"},
        }
    };
}

TEST_CASE (MultipleIndicesEditionAndDeletion)
{
    Scenario {
        Record::Reflection::GetMapping (),
        {
            OpenAllocator {},
            AllocateAndInit {&entity2Xavier},
            AllocateAndInit {&entity0Hugo},
            AllocateAndInit {&entity1Karl},
            AllocateAndInit {&entity1Karl},
            CloseAllocator {},

            CreateOrderedIndex {"entity", Record::Reflection::entityId},
            CreateOrderedIndex {"nickname", Record::Reflection::nickname},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity1Karl, &entity2Xavier}},
            CloseCursor {"allEntities"},

            OrderedIndexLookupToRead {{{"nickname", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all", {&entity0Hugo, &entity1Karl, &entity1Karl, &entity2Xavier}},
            CloseCursor {"all"},

            OrderedIndexLookupToEdit {{{"entity", "entity1"}, &Bounds::entity1, &Bounds::entity1}},
            CursorCheck {"entity1", &entity1Karl},
            CursorEdit {"entity1", &entity2Xavier},
            CursorDeleteRecord {"entity1"},
            CursorCheck {"entity1", &entity1Karl},
            CursorEdit {"entity1", &entity2Xavier},
            CloseCursor {"entity1"},

            OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity2Xavier, &entity2Xavier}},

            OrderedIndexLookupToRead {{{"nickname", "all"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"all", {&entity0Hugo, &entity2Xavier, &entity2Xavier}},
        }
    };
}

TEST_CASE (OrderingInt8)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::int8,
        {&firstRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes});
}

TEST_CASE (OrderingInt16)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::int16,
        {&secondRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes, &firstRecordWithAllFieldTypes});
}

TEST_CASE (OrderingInt32)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::int32,
        {&firstRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes});
}

TEST_CASE (OrderingInt64)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::int64,
        {&firstRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes});
}

TEST_CASE (OrderingUInt8)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::uint8,
        {&firstRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes});
}

TEST_CASE (OrderingUInt16)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::uint16,
        {&firstRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes});
}

TEST_CASE (OrderingUInt32)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::uint32,
        {&firstRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes});
}

TEST_CASE (OrderingUInt64)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::uint64,
        {&firstRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes});
}

TEST_CASE (OrderingFloat)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::floating,
        {&thirdRecordWithAllFieldTypes, &firstRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes});
}

TEST_CASE (OrderingDouble)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::doubleFloating,
        {&secondRecordWithAllFieldTypes, &firstRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes});
}

TEST_CASE (OrderingBlock)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::block,
        {&firstRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes});
}

TEST_CASE (OrderingString)
{
    ExecuteOrderingTest (
        AllFieldTypesStructure::Reflection::string,
        {&firstRecordWithAllFieldTypes, &secondRecordWithAllFieldTypes, &thirdRecordWithAllFieldTypes});
}

END_SUITE