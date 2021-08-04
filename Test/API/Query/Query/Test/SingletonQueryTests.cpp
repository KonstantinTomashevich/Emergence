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
                CursorCopy {"original", "copied"},
                CursorMove {"original", "moved"},
                CursorClose {"moved"},
                CursorClose {"copied"},

                QuerySingletonToEdit {{"singleton", "originalToEdit"}},
                CursorMove {"originalToEdit", "moved"},
                CursorClose {"moved"},

                QuerySingletonToRead {{"singleton", "another"}},
            }
        };
}
} // namespace Emergence::Query::Test::SingletonQuery
