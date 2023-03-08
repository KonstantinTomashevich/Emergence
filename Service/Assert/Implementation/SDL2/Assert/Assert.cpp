#include <Assert/Assert.hpp>

#include <Container/HashMap.hpp>
#include <Container/HashSet.hpp>

#include <debugbreak.h>

#include <Log/Log.hpp>

#include <SDL.h>

#include <stacktrace>

#include <SyntaxSugar/AtomicFlagGuard.hpp>

namespace Emergence
{
static bool interactiveAssertEnabled = false;
static std::atomic_flag interactiveAssertLock;

static Memory::Profiler::AllocationGroup allocationGroup {Memory::Profiler::AllocationGroup {
    Memory::Profiler::AllocationGroup::Root (), Memory::UniqueString {"InteractiveAssert"}}};

struct FileSkipData
{
    Container::HashSet<std::size_t> lines {allocationGroup};
};

static Container::HashMap<Container::String, FileSkipData> assertSkipsPerFile {allocationGroup};

void SetIsAssertInteractive (bool _interactive) noexcept
{
    interactiveAssertEnabled = _interactive;
}

void ReportCriticalError (const char *_expression, const char *_file, size_t _line) noexcept
{
    EMERGENCE_LOG (CRITICAL_ERROR, "Expression: ", _expression, ". File: ", _file, ". Line: ", _line);
    if (interactiveAssertEnabled)
    {
        AtomicFlagGuard interactiveAssertGuard {interactiveAssertLock};
        auto iterator = assertSkipsPerFile.find (_file);

        if (iterator != assertSkipsPerFile.end () && iterator->second.lines.contains (_line))
        {
            return;
        }

        constexpr int BUTTON_SKIP = 0;
        constexpr int BUTTON_SKIP_ALL_OCCURRENCES = 1;
        constexpr int BUTTON_BREAK_INTO_DEBUGGER = 2;
        constexpr int BUTTON_ABORT = 3;

        const SDL_MessageBoxButtonData buttons[] = {
            {0, BUTTON_ABORT, "Abort"},
            {0, BUTTON_BREAK_INTO_DEBUGGER, "Debug"},
            {0, BUTTON_SKIP_ALL_OCCURRENCES, "Skip all"},
            {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, BUTTON_SKIP, "Skip"},
        };

        Container::String assertText = EMERGENCE_BUILD_STRING ("Expression: ", _expression, ".\nFile: ", _file,
                                                                     ".\nLine: ", _line, ".\nStacktrace:\n");

        // We avoid using string builder here as stacktrace string is usually too big.
        assertText += std::to_string (std::stacktrace::current ());

        const SDL_MessageBoxData messageBoxData = {
            SDL_MESSAGEBOX_ERROR,    nullptr, "Assert failed!", assertText.c_str (),
            SDL_arraysize (buttons), buttons, nullptr};

        int resultButtonId;
        if (SDL_ShowMessageBox (&messageBoxData, &resultButtonId) < 0)
        {
            EMERGENCE_LOG (CRITICAL_ERROR, "Unable to show interactive assert message box!");
            std::abort ();
        }

        // If window was closed.
        if (resultButtonId < 0)
        {
            resultButtonId = BUTTON_SKIP;
        }

        if (resultButtonId == BUTTON_SKIP)
        {
            return;
        }

        if (resultButtonId == BUTTON_SKIP_ALL_OCCURRENCES)
        {
            assertSkipsPerFile[_file].lines.emplace (_line);
            return;
        }

        if (resultButtonId == BUTTON_BREAK_INTO_DEBUGGER)
        {
            debug_break ();
            return;
        }

        if (resultButtonId == BUTTON_ABORT)
        {
            std::abort ();
        }

        EMERGENCE_LOG (CRITICAL_ERROR, "Unknown interactive assert button: ", resultButtonId, "!");
        std::abort ();
    }
    else
    {
        std::abort ();
    }
}
} // namespace Emergence
