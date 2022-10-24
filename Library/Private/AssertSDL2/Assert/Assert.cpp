#include <Assert/Assert.hpp>

#include <Container/HashSet.hpp>

#include <debugbreak.h>

#include <Log/Log.hpp>

#include <SDL.h>

#include <SyntaxSugar/AtomicFlagGuard.hpp>

namespace Emergence
{
static bool interactiveAssertEnabled = false;
static std::atomic_flag interactiveAssertLock;
static Container::HashSet<Container::String> fileLinesToSkipAsserts {Memory::Profiler::AllocationGroup {
    Memory::Profiler::AllocationGroup::Root (), Memory::UniqueString {"InteractiveAssert"}}};

void SetIsAssertInteractive (bool _interactive) noexcept
{
    interactiveAssertEnabled = _interactive;
}

void AssertFailed (const char *_message, const char *_expression, const char *_file, size_t _line) noexcept
{
    EMERGENCE_LOG (CRITICAL_ERROR, "Assert failed: ", _message, ". Expression: ", _expression, ". File: ", _file,
                   ". Line: ", _line);
    if (interactiveAssertEnabled)
    {
        AtomicFlagGuard interactiveAssertGuard {interactiveAssertLock};
        const Container::String assertFileLine = EMERGENCE_BUILD_STRING (_file, "#L", _line);

        if (fileLinesToSkipAsserts.contains (assertFileLine))
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

        const Container::String assertText = EMERGENCE_BUILD_STRING (
            "Message: ", _message, ".\nExpression: ", _expression, ".\nFile: ", _file, ".\nLine: ", _line, ".");

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
            fileLinesToSkipAsserts.emplace (assertFileLine);
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
