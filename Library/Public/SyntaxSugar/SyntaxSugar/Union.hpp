#pragma once

#include <cassert>

namespace Emergence
{
/// \brief Creates non-copyable non-assignable union of two types with empty constructor and destructor.
/// \details Unions like this are useful in places where variants are bad due to performance (unnecessary branching,
///          exceptions, etc.) or space (unnecessary type index storage) reasons. Their usage provides developer
///          with opportunity to use hand-crafted optimizations, but requires extra caution.
#define EMERGENCE_UNION2(UnionName, Type0, Name0, Type1, Name1)                                                        \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    union UnionName                                                                                                    \
    {                                                                                                                  \
        /* Enforce manual construction. */                                                                             \
        UnionName ()                                                                                                   \
        {                                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        UnionName (const UnionName &_other) = delete;                                                                  \
                                                                                                                       \
        /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                   \
        UnionName (UnionName &&_other) = delete;                                                                       \
                                                                                                                       \
        /* Enforce manual destruction. */                                                                              \
        ~UnionName ()                                                                                                  \
        {                                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                   \
        UnionName &operator= (const UnionName &_other) = delete;                                                       \
                                                                                                                       \
        /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                   \
        UnionName &operator= (UnionName &&_other) = delete;                                                            \
                                                                                                                       \
        Type0 _0;                                                                                                      \
        Type0 Name0;                                                                                                   \
        Type1 _1;                                                                                                      \
        Type1 Name1;                                                                                                   \
    }

/// \brief Visitor for union, declared using EMERGENCE_UNION2.
/// \param _switch Index of currently used type (0 or 1).
template <typename ReturnType, typename Union, typename Functor>
ReturnType VisitUnion2 (Functor _functor, Union &_union, size_t _switch)
{
#define BODY(SwitchId)                                                                                                 \
    if constexpr (std::is_same_v<void, ReturnType>)                                                                    \
    {                                                                                                                  \
        _functor (_union._##SwitchId);                                                                                 \
        break;                                                                                                         \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        return _functor (_union._##SwitchId);                                                                          \
    }

    switch (_switch)
    {
    case 0u:
        BODY (0)

    default:
        assert (_switch == 1u);
        BODY (1)
    }

#undef BODY
};

/// \brief Shortcut for calling method through VisitUnion2.
#define EMERGENCE_UNION2_CALL(UnionFieldName, SwitchFieldName, ReturnType, MethodName, ...)                            \
    Emergence::VisitUnion2<ReturnType> (                                                                               \
        [&] (auto &_value)                                                                                             \
        {                                                                                                              \
            return _value.MethodName (__VA_ARGS__);                                                                    \
        },                                                                                                             \
        UnionFieldName, SwitchFieldName)

/// \brief Extends visitation behaviour of VisitUnion2 for calling any function based on switch value.
/// \details Useful in pair with two-value unions, in cases when we don't really need to access union value,
///          but need to call other method depending on current union type.
#define EMERGENCE_UNION2_SWITCH_CALL(SwitchSource, CallTarget, ...)                                                    \
    [&] ()                                                                                                             \
    {                                                                                                                  \
        switch (SwitchSource)                                                                                          \
        {                                                                                                              \
        case 0u:                                                                                                       \
            return CallTarget##0(__VA_ARGS__);                                                                         \
        default:                                                                                                       \
            assert (SwitchSource == 1u);                                                                               \
            return CallTarget##1(__VA_ARGS__);                                                                         \
        }                                                                                                              \
    }()

/// \brief Shortcut for calling destructor through VisitUnion2.
#define EMERGENCE_UNION2_DESTRUCT(UnionFieldName, SwitchFieldName)                                                     \
    Emergence::VisitUnion2<void> (                                                                                     \
        [&] (auto &_value)                                                                                             \
        {                                                                                                              \
            using Type = std::decay_t<decltype (_value)>;                                                              \
            _value.~Type ();                                                                                           \
        },                                                                                                             \
        UnionFieldName, SwitchFieldName)

} // namespace Emergence
