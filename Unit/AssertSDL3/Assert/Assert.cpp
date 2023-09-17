#include <Assert/Assert.hpp>

#include <Container/HashMap.hpp>
#include <Container/HashSet.hpp>

#include <debugbreak.h>

#include <Log/Log.hpp>

#include <SDL3/SDL.h>

#if !defined(EMERGENCE_FALLBACK_TO_CXX_20)
#    include <stacktrace>
#endif

#include <Threading/AtomicFlagGuard.hpp>

namespace Emergence
{
namespace
{
bool &IsInteractiveAssertEnabled ()
{
    static bool interactiveAssertEnabled = false;
    return interactiveAssertEnabled;
}

std::atomic_flag &GetInteractiveAssertLock ()
{
    static std::atomic_flag interactiveAssertLock;
    return interactiveAssertLock;
}

Memory::Profiler::AllocationGroup &GetInteractiveAssertAllocationGroup ()
{
    static Memory::Profiler::AllocationGroup allocationGroup {Memory::Profiler::AllocationGroup {
        Memory::Profiler::AllocationGroup::Root (), Memory::UniqueString {"InteractiveAssert"}}};
    return allocationGroup;
}

struct FileSkipData
{
    Container::HashSet<std::size_t> lines {GetInteractiveAssertAllocationGroup ()};
};

Container::HashMap<Container::String, FileSkipData> &GetAssertSkipsPerFile ()
{
    static Container::HashMap<Container::String, FileSkipData> assertSkipsPerFile {
        GetInteractiveAssertAllocationGroup ()};
    return assertSkipsPerFile;
}

} // namespace

void SetIsAssertInteractive (bool _interactive) noexcept
{
    IsInteractiveAssertEnabled () = _interactive;
}

void ReportCriticalError (const char *_expression, const char *_file, std::size_t _line) noexcept
{
    EMERGENCE_LOG (CRITICAL_ERROR, "Expression: ", _expression, ". File: ", _file, ". Line: ", _line);
#if !defined(EMERGENCE_FALLBACK_TO_CXX_20)
    for (const std::stacktrace_entry &entry : std::stacktrace::current ())
    {
        EMERGENCE_LOG (CRITICAL_ERROR, std::to_string (entry).c_str ());
    }
#endif

    if (IsInteractiveAssertEnabled ())
    {
        AtomicFlagGuard interactiveAssertGuard {GetInteractiveAssertLock ()};
        auto iterator = GetAssertSkipsPerFile ().find (_file);

        if (iterator != GetAssertSkipsPerFile ().end () && iterator->second.lines.contains (_line))
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

#if !defined(EMERGENCE_FALLBACK_TO_CXX_20)
        // We avoid using string builder here as stacktrace string is usually too big.
        assertText += std::to_string (std::stacktrace::current ());
#endif

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
            GetAssertSkipsPerFile ()[_file].lines.emplace (_line);
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
