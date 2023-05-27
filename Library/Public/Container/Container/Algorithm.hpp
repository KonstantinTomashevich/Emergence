#pragma once

namespace Emergence::Container
{
/// \brief Some STL implementation have strict requirements for find_if
///        iterators, therefore we have our less strict find_if version.
template <typename Iterator, typename Predicate>
Iterator FindIf (Iterator _begin, Iterator _end, Predicate _predicate) noexcept
{
    while (_begin != _end)
    {
        if (_predicate (*_begin))
        {
            return _begin;
        }

        ++_begin;
    }

    return _begin;
}
} // namespace Emergence::Container
