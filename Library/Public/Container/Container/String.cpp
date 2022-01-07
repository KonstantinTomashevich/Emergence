#include <Container/String.hpp>

namespace Emergence
{
namespace Container
{
// TODO: Temporary we are just copying results from std::to_string. Should be rewritten later.
String ToString (int _value)
{
    return std::to_string (_value).c_str ();
}

String ToString (long _value)
{
    return std::to_string (_value).c_str ();
}

String ToString (long long int _value)
{
    return std::to_string (_value).c_str ();
}

String ToString (unsigned int _value)
{
    return std::to_string (_value).c_str ();
}

String ToString (unsigned long _value)
{
    return std::to_string (_value).c_str ();
}

String ToString (unsigned long long int _value)
{
    return std::to_string (_value).c_str ();
}

String ToString (float _value)
{
    return std::to_string (_value).c_str ();
}

String ToString (double _value)
{
    return std::to_string (_value).c_str ();
}

String ToString (long double _value)
{
    return std::to_string (_value).c_str ();
}
} // namespace Container

namespace Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<char>::Get () noexcept
{
    static Profiler::AllocationGroup group {Profiler::AllocationGroup::Root (), UniqueString {"UntrackedString"}};
    return group;
}
} // namespace Memory
} // namespace Emergence
