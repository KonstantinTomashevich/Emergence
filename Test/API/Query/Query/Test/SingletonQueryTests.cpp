#include <Query/Test/Data.hpp>
#include <Query/Test/SingletonQueryTests.hpp>

namespace Emergence::Query::Test::SingletonQuery
{
using namespace Tasks;

Storage RequestStorage ()
{
    return
        {
            Player::Reflection::GetMapping (),
            {&HUGO_0_ALIVE_STUNNED},
            {Sources::Singleton {"singleton"}}
        };
}

Scenario EditAndRead () noexcept
{
    return
        {
            {
                RequestStorage (),
            },
            {
                QuerySingletonToEdit {{"singleton", "singleton"}},
                CursorCheck {"singleton", &HUGO_0_ALIVE_STUNNED},
                CursorEdit {"singleton", &KARL_1_ALIVE_IMMOBILIZED},
                CursorClose {"singleton"},

                QuerySingletonToRead {{"singleton", "singleton"}},
                CursorCheck {"singleton", &KARL_1_ALIVE_IMMOBILIZED},
                CursorClose {"singleton"},
            }
        };
}

Scenario CursorManipulations () noexcept
{
    return
        {
            {
                RequestStorage (),
            },
            {
                QuerySingletonToRead {{"singleton", "original"}},
                CursorCheck {"original", &HUGO_0_ALIVE_STUNNED},

                CursorCopy {"original", "copied"},
                CursorCheck {"copied", &HUGO_0_ALIVE_STUNNED},

                CursorMove {"original", "moved"},
                CursorCheck {"moved", &HUGO_0_ALIVE_STUNNED},

                CursorClose {"moved"},
                CursorCheck {"copied", &HUGO_0_ALIVE_STUNNED},
                CursorClose {"copied"},

                QuerySingletonToEdit {{"singleton", "originalToEdit"}},
                CursorCheck {"originalToEdit", &HUGO_0_ALIVE_STUNNED},

                CursorMove {"originalToEdit", "moved"},
                CursorCheck {"moved", &HUGO_0_ALIVE_STUNNED},
                CursorClose {"moved"},

                QuerySingletonToRead {{"singleton", "another"}},
                CursorCheck {"another", &HUGO_0_ALIVE_STUNNED},
            }
        };
}
} // namespace Emergence::Query::Test::SingletonQuery
