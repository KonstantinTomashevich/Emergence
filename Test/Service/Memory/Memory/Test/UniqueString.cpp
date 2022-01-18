#define _CRT_SECURE_NO_WARNINGS

#include <cstring>
#include <string_view>

#include <Memory/Test/UniqueString.hpp>
#include <Memory/UniqueString.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Test
{
bool UniqueStringTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Memory::Test

BEGIN_SUITE (UniqueString)

TEST_CASE (Construction)
{
    const std::string_view initialString = "Hello, world!";
    Emergence::Memory::UniqueString uniqueString {initialString};
    CHECK (initialString.data () != *uniqueString);
    CHECK (strcmp (initialString.data (), *uniqueString) == 0);
}

TEST_CASE (DiffrentStrings)
{
    Emergence::Memory::UniqueString first {"First!"};
    Emergence::Memory::UniqueString second {"Second!"};

    CHECK (first != second);
    CHECK (first == first);
    CHECK (second == second);
}

TEST_CASE (EqualStrings)
{
    Emergence::Memory::UniqueString first {"Hello, world!"};
    Emergence::Memory::UniqueString second {"Hello, world!"};
    CHECK (first == second);
}

TEST_CASE (Empty)
{
    Emergence::Memory::UniqueString empty;
    CHECK (!*empty);
    CHECK (empty == empty);
}

TEST_CASE (Hash)
{
    using namespace Emergence::Memory::Literals;

    Emergence::Memory::UniqueString first {"First!"};
    Emergence::Memory::UniqueString second {"Second!"};
    Emergence::Memory::UniqueString firstAgain = "First!"_us;

    CHECK (first.Hash () != second.Hash ());
    CHECK (first.Hash () == firstAgain.Hash ());
    CHECK (second.Hash () != firstAgain.Hash ());

    CHECK (std::hash<Emergence::Memory::UniqueString> {}(first) ==
           std::hash<Emergence::Memory::UniqueString> {}(firstAgain));
}

END_SUITE
