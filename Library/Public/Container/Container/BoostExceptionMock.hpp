#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

#include <Assert/Assert.hpp>

#include <boost/throw_exception.hpp>

namespace boost
{
// We do not support exceptions, therefore we need to provide mock for boost exceptions.
BEGIN_MUTING_NO_RETURN_WARNINGS
BOOST_NORETURN inline void throw_exception (const std::exception &_exception)
{
    Emergence::ReportCriticalError (_exception.what (), __FILE__, __LINE__);
#if defined(__GNUC__)
    // Old GCC versions have no flag to mute no-return warning.
    throw;
#endif
}
END_MUTING_WARNINGS
} // namespace boost
