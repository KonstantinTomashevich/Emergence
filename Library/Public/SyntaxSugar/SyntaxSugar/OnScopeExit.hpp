#pragma once

namespace Emergence
{
/// \brief Executes stored lambda on destruction.
template <typename Lambda>
class ScopeExitGuard final
{
public:
    ScopeExitGuard (Lambda _lambda) noexcept
        : lambda (std::move (_lambda))
    {
    }

    ScopeExitGuard (const ScopeExitGuard &_other) = delete;

    ScopeExitGuard (ScopeExitGuard &&_other) = delete;

    ~ScopeExitGuard ()
    {
        lambda ();
    }

    ScopeExitGuard &operator= (const ScopeExitGuard &_other) = delete;

    ScopeExitGuard &operator= (ScopeExitGuard &&_other) = delete;

private:
    Lambda lambda;
};
} // namespace Emergence

/// \brief Executes given lambda on scope exit.
/// \invariant Having more than one OnScopeExit hook is not supported.
#define EMERGENCE_ON_SCOPE_EXIT(...)                                                                                   \
    auto onScopeExitLambda = __VA_ARGS__;                                                                              \
    Emergence::ScopeExitGuard<decltype (onScopeExitLambda)> scopeExitGuard                                             \
    {                                                                                                                  \
        std::move (onScopeExitLambda)                                                                                  \
    }