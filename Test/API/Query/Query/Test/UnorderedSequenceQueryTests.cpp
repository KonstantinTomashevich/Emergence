#include <Query/Test/Common.hpp>
#include <Query/Test/Data.hpp>
#include <Query/Test/UnorderedSequenceQueryTests.hpp>

namespace Emergence::Query::Test::UnorderedSequenceQuery
{
using namespace Tasks;

Storage RequestStorage (const std::vector <const void *> &_objects)
{
    return
        {
            Player::Reflection::GetMapping (),
            _objects,
            {Sources::UnorderedSequence {"sequence"}}
        };
}

Scenario CursorManipulations () noexcept
{
    return
        {
            {
                RequestStorage ({&HUGO_0_ALIVE_STUNNED}),
            },
            TestCursorCopyAndMove (
                QueryUnorderedSequenceToRead {{"sequence", "cursor"}},
                QueryUnorderedSequenceToEdit {{"sequence", "cursor"}},
                &HUGO_0_ALIVE_STUNNED, nullptr, &HUGO_0_ALIVE_STUNNED, nullptr)
        };
}

Scenario Read () noexcept
{
    return
        {
            {
                RequestStorage ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}),
            },
            {
                QueryUnorderedSequenceToRead {{"sequence", "cursor"}},
                CursorCheckAllUnordered
                    {
                        "cursor",
                        {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}
                    }
            }
        };
}

Scenario Edit () noexcept
{
    return
        {
            {
                RequestStorage ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}),
            },
            {
                QueryUnorderedSequenceToEdit {{"sequence", "cursor"}},
                CursorEdit {"cursor", &KARL_0_ALIVE_IMMOBILIZED},
                CursorIncrement {"cursor"},
                CursorEdit {"cursor", &KARL_0_ALIVE_IMMOBILIZED},
                CursorIncrement {"cursor"},
                CursorEdit {"cursor", &XAVIER_2_ALIVE_POISONED},
                CursorIncrement {"cursor"},
                CursorCheck {"cursor", nullptr},
                CursorClose {"cursor"},

                QueryUnorderedSequenceToRead {{"sequence", "cursor"}},
                CursorCheckAllUnordered
                    {
                        "cursor",
                        {&KARL_0_ALIVE_IMMOBILIZED, &KARL_0_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}
                    }
            }
        };
}

Scenario Delete () noexcept
{
    return
        {
            {
                RequestStorage ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}),
            },
            {
                QueryUnorderedSequenceToEdit {{"sequence", "cursor"}},
                CursorDeleteObject {"cursor"},
                CursorDeleteObject {"cursor"},
                CursorDeleteObject {"cursor"},
                CursorCheck {"cursor", nullptr},
                CursorClose {"cursor"},

                QueryUnorderedSequenceToRead {{"sequence", "cursor"}},
                CursorCheckAllUnordered {"cursor", {}}
            }
        };
}

Scenario EditAndDelete () noexcept
{
    return
        {
            {
                RequestStorage ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &XAVIER_2_ALIVE_POISONED}),
            },
            {
                QueryUnorderedSequenceToEdit {{"sequence", "cursor"}},
                CursorEdit {"cursor", &KARL_0_ALIVE_IMMOBILIZED},
                CursorIncrement {"cursor"},
                CursorEdit {"cursor", &XAVIER_2_ALIVE_POISONED},
                CursorDeleteObject {"cursor"},
                CursorEdit {"cursor", &HUGO_0_ALIVE_STUNNED},
                CursorIncrement {"cursor"},
                CursorCheck {"cursor", nullptr},
                CursorClose {"cursor"},

                QueryUnorderedSequenceToRead {{"sequence", "cursor"}},
                CursorCheckAllUnordered {"cursor", {&HUGO_0_ALIVE_STUNNED, &KARL_0_ALIVE_IMMOBILIZED}}
            }
        };
}
} // namespace Emergence::Query::Test::UnorderedSequenceQuery